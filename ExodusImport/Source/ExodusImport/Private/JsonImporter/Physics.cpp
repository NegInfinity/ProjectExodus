#include "JsonImportPrivatePCH.h"
#include "JsonImporter.h"

#include "UnrealUtilities.h"
#include "JsonObjects.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#define LOCTEXT_NAMESPACE "FExodusImportModule"

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

		if (curJoint.isFixedJointType()){
			physConstraint->SetLinearXLimit(LCM_Locked, 1.0f);
			physConstraint->SetLinearYLimit(LCM_Locked, 1.0f);
			physConstraint->SetLinearZLimit(LCM_Locked, 1.0f);
			physConstraint->SetAngularSwing1Limit(ACM_Locked, 0.0f);
			physConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
			physConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);
		}
		else if (curJoint.isHingeJointType()){
			physConstraint->SetLinearXLimit(LCM_Locked, 1.0f);
			physConstraint->SetLinearYLimit(LCM_Locked, 1.0f);
			physConstraint->SetLinearZLimit(LCM_Locked, 1.0f);
			physConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
			physConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);
			physConstraint->SetAngularSwing1Limit(ACM_Free, 0.0f);

			if (curJoint.hingeJointData.Num() == 0){
				UE_LOG(JsonLog, Warning, TEXT("Hinge joint data not found on joint %d, obj %d (%s)"), jointIndex, obj.id, *obj.name);
			}
			else{
				//UE_LOG(JsonLog, Warning, TEXT("%f %f %f"), curJoint.anchor.X, curJoint.anchor.Y, curJoint.anchor.Z);
				FVector jointPos = obj.unityLocalPosToUnrealWorld(curJoint.anchor);
				//UE_LOG(JsonLog, Warning, TEXT("%f %f %f"), jointPos.X, jointPos.Y, jointPos.Z);

				FVector jointAxis = obj.unityLocalVectorToUnrealWorld(curJoint.axis);//This is actually z axis, meaning up. X-->forward Y-->right, Z --> up
				UE_LOG(JsonLog, Log, TEXT("Joint axis: %f %f %f; unrealAxis: %f %f %f"),
					curJoint.axis.X, curJoint.axis.Y, curJoint.axis.Z,
					jointAxis.X, jointAxis.Y, jointAxis.Z);
				FVector jointSwing1;
				FVector jointSwing2;

				FTransform hingeTransform;

				auto hingePos = jointPos;
				auto hingeZ = jointAxis;
				auto hingeX = makePerpendicular(hingeZ);
				auto hingeY = FVector::CrossProduct(hingeX, hingeZ);
				hingeY.Normalize();

				FMatrix hingeMatrix = FMatrix::Identity;//Goddamit, again. Partially initialized matrix.
				hingeMatrix.SetAxes(&hingeX, &hingeY, &hingeZ, &hingePos);

				jointTransform.SetFromMatrix(hingeMatrix);

				const auto& hingeData = curJoint.hingeJointData[0];

				physConstraint->SetWorldTransform(hingeTransform);
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

