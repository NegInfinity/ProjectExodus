#include "JsonImportPrivatePCH.h"
#include "JointBuilder.h"
#include "JsonGameObjectRegistry.h"

#include "UnrealUtilities.h"
#include "JsonObjects.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#define LOCTEXT_NAMESPACE "FExodusImportModule"

const bool JointBuilder::isSupportedJoint(const JsonPhysicsJoint &joint) const{
	return joint.isFixedJointType()
		|| joint.isHingeJointType();
}

void JointBuilder::setupFixedJointConstraint(UPhysicsConstraintComponent *physConstraint, const JsonPhysicsJoint &joint) const{
	check(physConstraint);
	physConstraint->SetLinearXLimit(LCM_Locked, 1.0f);
	physConstraint->SetLinearYLimit(LCM_Locked, 1.0f);
	physConstraint->SetLinearZLimit(LCM_Locked, 1.0f);
	physConstraint->SetAngularSwing1Limit(ACM_Locked, 0.0f);
	physConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
	physConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);
}

void JointBuilder::setupHingeJointConstraint(UPhysicsConstraintComponent *physConstraint, int jointIndex,
FTransform &jointTransform, const JsonPhysicsJoint &joint, const JsonGameObject &obj) const{
	using namespace UnrealUtilities;

	//We're using Swing1 axis for hinges. WHich is Z-axis
	physConstraint->SetLinearXLimit(LCM_Locked, 1.0f);
	physConstraint->SetLinearYLimit(LCM_Locked, 1.0f);
	physConstraint->SetLinearZLimit(LCM_Locked, 1.0f);
	physConstraint->SetAngularSwing1Limit(ACM_Free, 0.0f);
	physConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
	physConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);

	if (joint.hingeJointData.Num() == 0){
		UE_LOG(JsonLog, Warning, TEXT("Hinge joint data not found on joint %d, obj %d (%s)"), jointIndex, obj.id, *obj.name);
		return;
	}

	FVector jointPos = obj.unityLocalPosToUnrealWorld(joint.anchor);
	FVector jointAxis = obj.unityLocalVectorToUnrealWorld(joint.axis);//This is actually z axis, meaning up. X-->forward Y-->right, Z --> up
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

	float midAngle = 0.0f;//Affected by limits, "middle" of rotational block.

	const auto& hingeData = joint.hingeJointData[0];
	if (hingeData.useLimits){
		float angleRange = hingeData.limits.max - hingeData.limits.min;
		float symmetricAngleLimit = angleRange * 0.5f;
		midAngle = (hingeData.limits.max + hingeData.limits.min) * 0.5f;
		physConstraint->SetAngularSwing1Limit(ACM_Limited, symmetricAngleLimit);

		if (!FMath::IsNearlyZero(midAngle)){
			float rotationAngle = midAngle;
			auto constraintOffsetRotator = FRotator(0.0f, midAngle, 0.0f);
			physConstraint->ConstraintInstance.AngularRotationOffset += constraintOffsetRotator;				
		}
	}

	FMatrix hingeMatrix = FMatrix::Identity;//Goddamit, again. Partially initialized matrix.
	hingeMatrix.SetAxes(&hingeX, &hingeY, &hingeZ, &hingePos);

	jointTransform.SetFromMatrix(hingeMatrix);

	if (hingeData.useMotor && hingeData.useSpring){
		UE_LOG(JsonLog, Warning, TEXT("Both spring and motor are enabled on object %d(%s), jointIndex %d. Only spring will be active"), obj.id, *obj.name, jointIndex);
	}

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
		physConstraint->SetAngularDriveParams(0.0f, unityAngularMotorForceToUnreal(hingeData.motor.force), 0.0f);
		//hingeData.motor.freeSpin; //? There doesn't seem to be an equivalent
	}

	if (hingeData.useSpring){
		physConstraint->SetAngularDriveMode(EAngularDriveMode::TwistAndSwing);
		physConstraint->SetAngularOrientationDrive(true, false);

		auto orientRotator = FRotator(0.0f, hingeData.spring.targetPosition - midAngle, 0.0f);
		physConstraint->SetAngularOrientationTarget(orientRotator);
		physConstraint->SetAngularDriveParams(unityAngularSpringForceToUnreal(hingeData.spring.spring), 0.0f, 0.0f);
	}
	//physConstraint->SetWorldTransform(hingeTransform);
}

void JointBuilder::processPhysicsJoint(const JsonGameObject &obj, const InstanceIdMap &instanceMap,
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

		if (!isSupportedJoint(curJoint)	){
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
			setupFixedJointConstraint(physConstraint, curJoint);
		}
		else if (curJoint.isHingeJointType()){
			setupHingeJointConstraint(physConstraint, jointIndex, jointTransform, curJoint, obj);
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

void JointBuilder::processPhysicsJoints(const TArray<JsonGameObject>& objects, ImportWorkData &workData) const{
	InstanceIdMap instanceMap;
	FScopedSlowTask progress(objects.Num(), LOCTEXT("Processing joints", "Processing joints"));
	progress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing joints"));
	for (int i = 0; i < objects.Num(); i++){
		auto srcObj = objects[i];
		if (srcObj.hasJoints()){
			if (instanceMap.isEmpty()){
				buildInstanceIdMap(instanceMap, objects);
				check(!instanceMap.isEmpty());
			}
			processPhysicsJoint(srcObj, instanceMap, objects, workData);
		}
		progress.EnterProgressFrame(1.0f);
	}
}

void JointBuilder::buildInstanceIdMap(InstanceIdMap &outMap, const TArray<JsonGameObject>& objects){
	outMap.clear();
	for (const auto &cur : objects){
		auto instId = cur.instanceId;
		auto objId = cur.id;
		outMap.registerId(instId, objId);
	}
}

const JsonGameObject* JointBuilder::resolveObjectReference(const JsonObjectReference &ref,
	const InstanceIdMap &instanceMap, const TArray<JsonGameObject>& objects) const{
	if (ref.isNull)
		return nullptr;
	auto found = instanceMap.find(ref.instanceId);
	if (!found)
		return nullptr;
	if ((*found < 0) || (*found >= objects.Num()))
		return nullptr;
	return &objects[*found];
}
