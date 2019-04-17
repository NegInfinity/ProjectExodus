#include "JsonImportPrivatePCH.h"
#include "JsonImporter.h"

#include "UnrealUtilities.h"
#include "JsonObjects.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#define LOCTEXT_NAMESPACE "FExodusImportModule"

#if 0
void JsonImporter::processPhysicsJoint(const JsonGameObject &obj, const InstanceToIdMap &instanceMap,
	const TArray<JsonGameObject>& objects, ImportWorkData &workData) const{
	using namespace UnrealUtilities;

	if (!obj.hasJoints())
		return;
	auto srcObj = workData.findImportedObject(obj.id);
	if (!srcObj){
		UE_LOG(JsonLog, Warning, TEXT("Src object %d not found while processing joints"), obj.id);
	}

	check(srcObj->hasComponent() || srcObj->hasActor());
	auto srcRootActor = srcObj->findRootActor();
	check(srcRootActor);
	for (int jointIndex = 0; jointIndex < obj.joints.Num(); jointIndex++){
		const JsonPhysicsJoint &curJoint = obj.joints[jointIndex];

		auto dstJsonObj = resolveObjectReference(curJoint.connectedBodyObject, instanceMap, objects);
		if (!curJoint.isConnectedToWorld() && !dstJsonObj){
			UE_LOG(JsonLog, Warning, TEXT("dst object %d not found while processing joints on %d(%d: \"%s\")"),
				curJoint.connectedBodyObject.instanceId, obj.instanceId, obj.id, *obj.name);
			continue;
		}

		if (curJoint.isSpringJointType()
			//|| curJoint.isHingeJointType()
			|| curJoint.isConfigurableJointType() 
			|| curJoint.isConfigurableJointType()
			){
			UE_LOG(JsonLog, Warning, TEXT("Unsupported joint type %s at object %d(%s)"),
				*curJoint.jointType, obj.id, *obj.name);
			continue;
		}

		AActor *dstActor = nullptr;
		UPrimitiveComponent *dstComponent = nullptr;
		if (dstJsonObj){
			auto dstObj = workData.findImportedObject(dstJsonObj->id);
			if (dstObj){
				dstActor = dstObj->findRootActor();
				dstComponent = Cast<UPrimitiveComponent>(dstObj->component);
			}
		}

		auto physConstraint = NewObject<UPhysicsConstraintComponent>(srcRootActor);
		auto anchorPos = curJoint.anchor;

		auto jointTransform = obj.getUnrealTransform(anchorPos);
		auto physObj = ImportedObject(physConstraint);

		auto jointName = FString::Printf(TEXT("joint_%d_%s"), jointIndex, *curJoint.jointType);
		physObj.setNameOrLabel(jointName);
		//physConstraint->RegisterComponent();

		physConstraint->SetLinearBreakable(curJoint.isLinearBreakable(), unityForceToUnreal(curJoint.breakForce));
		physConstraint->SetAngularBreakable(curJoint.isAngularBreakable(), unityTorqueToUnreal(curJoint.breakTorque));
		UE_LOG(JsonLog, Log, TEXT("linear breakable: %d (%f); angular breakable: %d (%f);"),
			(int)curJoint.isLinearBreakable(), curJoint.breakForce,
			(int)curJoint.isAngularBreakable(), curJoint.breakTorque);

		auto srcActor = srcObj->findRootActor();
		auto srcComponent = Cast<UPrimitiveComponent>(srcObj->component);

		physConstraint->ConstraintActor1 = srcActor;
		if (srcComponent)
			physConstraint->OverrideComponent1 = srcComponent;
		physConstraint->ConstraintActor2 = dstActor;
		if (dstComponent)
			physConstraint->OverrideComponent2 = dstComponent;

		physConstraint->SetDisableCollision(!curJoint.enableCollision);

		if (curJoint.isFixedJointType()){
			physConstraint->SetLinearXLimit(LCM_Locked, 1.0f);
			physConstraint->SetLinearYLimit(LCM_Locked, 1.0f);
			physConstraint->SetLinearZLimit(LCM_Locked, 1.0f);
			physConstraint->SetAngularSwing1Limit(ACM_Locked, 0.0f);
			physConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
			physConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);
		}
		else if (curJoint.isHingeJointType()){
			//We're using Swing1 axis for hinges. WHich is Z-axis
			physConstraint->SetLinearXLimit(LCM_Locked, 1.0f);
			physConstraint->SetLinearYLimit(LCM_Locked, 1.0f);
			physConstraint->SetLinearZLimit(LCM_Locked, 1.0f);
			physConstraint->SetAngularSwing1Limit(ACM_Free, 0.0f);
			physConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
			physConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);

			if (curJoint.hingeJointData.Num() == 0){
				UE_LOG(JsonLog, Warning, TEXT("Hinge joint data not found on joint %d, obj %d (%s)"), jointIndex, obj.id, *obj.name);
			}
			else{
				FVector jointPos = obj.unityLocalPosToUnrealWorld(curJoint.anchor);
				FVector jointAxis = obj.unityLocalVectorToUnrealWorld(curJoint.axis);//This is actually z axis, meaning up. X-->forward Y-->right, Z --> up
				FTransform hingeTransform;

				auto objectUp = obj.unityLocalVectorToUnrealWorld(getUnityUpVector());//unity's up vector

				auto hingePos = jointPos;
				auto hingeZ = jointAxis;
				auto hingeY = makePerpendicular(hingeZ, objectUp);
				//auto hingeX = makePerpendicular(hingeZ);
				//auto hingeY = FVector::CrossProduct(hingeX, hingeZ);
				//hingeY.Normalize();
				auto hingeX = FVector::CrossProduct(hingeY, hingeZ);
				hingeX.Normalize();

				const auto& hingeData = curJoint.hingeJointData[0];
				if (hingeData.useLimits){
					float angleRange = hingeData.limits.max - hingeData.limits.min;
					float symmetricAngleLimit = angleRange * 0.5f;
					float midAngle = (hingeData.limits.max + hingeData.limits.max) * 0.5f;
					physConstraint->SetAngularSwing1Limit(ACM_Limited, symmetricAngleLimit);

					//This doens't work, as even if we rotate the joint, it spawns rotated at the MIDDLE of the range. 
					//Looking for angular rotation offset
					/*
					if (!FMath::IsNearlyZero(midAngle)){
						float rotationAngle = midAngle;// -hingeData.limits.min;
						//float fSin, fCos;
						float angleRad = FMath::DegreesToRadians(rotationAngle);

						float fSin = sinf(angleRad);
						float fCos = cosf(angleRad);

						//FMath::SinCos(&fSin, &fCos, FMath::DegreesToRadians(rotationAngle)); approximation...

						auto hingeX1 = hingeX * fCos + hingeY * fSin;
						auto hingeY1 = hingeY * fCos - hingeX * fSin;
						hingeX1.Normalize();
						hingeY1.Normalize();

						hingeX = hingeX1;
						hingeY = hingeY1;
					}
					*/
				}

				FMatrix hingeMatrix = FMatrix::Identity;//Goddamit, again. Partially initialized matrix.
				hingeMatrix.SetAxes(&hingeX, &hingeY, &hingeZ, &hingePos);

				jointTransform.SetFromMatrix(hingeMatrix);

				if (hingeData.useMotor){
					physConstraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
					physConstraint->SetAngularVelocityDrive(true, false);

					//Looks like angular rotration needs to be inverted
					//hingeData.motor.force;
					auto angularVelTarget = FVector(0.0f, 0.0f, -unityAngularVelocityToUe(hingeData.motor.targetVelocity));
					physConstraint->SetAngularVelocityTarget(angularVelTarget);
					//Second parameter refers to strength.
					//Last one - to max force.
					//hingeData.motor.force;
					physConstraint->SetAngularDriveParams(0.0f, unityForceToUnreal(hingeData.motor.force), 0.0f);
					//hingeData.motor.freeSpin; //? There doesn't seem to be an equivalent
				}

				if (hingeData.useSpring){
				}


				//physConstraint->SetWorldTransform(hingeTransform);
			}
		}
		else{
			UE_LOG(JsonLog, Warning, TEXT("Unhandled joint type %s at object %d(%s)"),
				*curJoint.jointType, obj.id, *obj.name);
		}

		physConstraint->SetWorldTransform(jointTransform);
		physObj.attachTo(srcObj);
		physObj.convertToInstanceComponent();
		physObj.fixEditorVisibility();
	}
}

void JsonImporter::processPhysicsJoints(const TArray<JsonGameObject>& objects, ImportWorkData &workData) const{
	InstanceToIdMap instanceMap;
	FScopedSlowTask progress(objects.Num(), LOCTEXT("Processing joints", "Processing joints"));
	progress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing joints"));
	for (int i = 0; i < objects.Num(); i++){
		auto srcObj = objects[i];
		if (srcObj.hasJoints()){
			if (instanceMap.Num() == 0){
				buildInstanceIdMap(instanceMap, objects);
				check(instanceMap.Num() != 0);//the map can't be empty past this point, at least one entry should be there...
			}
			processPhysicsJoint(srcObj, instanceMap, objects, workData);
		}
		progress.EnterProgressFrame(1.0f);
	}
}

#endif