#include "JsonImportPrivatePCH.h"
#include "JointBuilder.h"
#include "JsonGameObjectRegistry.h"

#include "UnrealUtilities.h"
#include "JsonObjects.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#define LOCTEXT_NAMESPACE "FExodusImportModule"

const bool JointBuilder::isSupportedJoint(const JsonPhysicsJoint &joint) const{
	return joint.isFixedJointType()
		|| joint.isHingeJointType()
		|| joint.isSpringJointType()
		|| joint.isCharacterJointType()
		|| joint.isConfigurableJointType()
	;
}

void JointBuilder::setupConfigurableJointConstraint(UPhysicsConstraintComponent *physConstraint, int jointIndex, FTransform &jointTransform,
const JsonPhysicsJoint &joint, const JsonGameObject &jointObject) const{
	using namespace UnrealUtilities;
	check(physConstraint);

	physConstraint->SetLinearXLimit(LCM_Locked, 1.0f);
	physConstraint->SetLinearYLimit(LCM_Locked, 1.0f);
	physConstraint->SetLinearZLimit(LCM_Locked, 1.0f);
	physConstraint->SetAngularSwing1Limit(ACM_Locked, 0.0f);
	physConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
	physConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);

	if (joint.configurableJointData.Num() == 0){
		UE_LOG(JsonLog, Warning, TEXT("Spring joint data not found on joint %d, obj %d (%s)"), jointIndex, jointObject.id, *jointObject.name);
		return;
	}
	const auto& confData = joint.configurableJointData[0];
	/*
		Swing1 is XY plane rotation
		Swing2 is XZ plane.
		Twist is YZ.

		Z is swing1
		Y is swing2
		X is twist
		Those are unreal coordinates/axes

		Now.... in unreal components, 
		X is forward, Y right, Z is up.

		In unity X is right, Z is forward, Y is up.
	*/

	auto unityAngularX = getAngularMotionChecked(confData.angularXMotion, "angularXMotion", jointIndex, jointObject);
	auto unityAngularY = getAngularMotionChecked(confData.angularYMotion, "angularYMotion", jointIndex, jointObject);
	auto unityAngularZ = getAngularMotionChecked(confData.angularZMotion, "angularZMotion", jointIndex, jointObject);
	auto unityLinearX = getLinearMotionChecked(confData.xMotion, "xMotion", jointIndex, jointObject);
	auto unityLinearY = getLinearMotionChecked(confData.yMotion, "yMotion", jointIndex, jointObject);
	auto unityLinearZ = getLinearMotionChecked(confData.zMotion, "zMotion", jointIndex, jointObject);

	auto unityPrimaryAxis = jointObject.unityLocalVectorToUnrealWorld(joint.axis);
	auto unitySecondaryAxis = jointObject.unityLocalVectorToUnrealWorld(confData.secondaryAxis);

	unityPrimaryAxis.Normalize();
	unitySecondaryAxis.Normalize();

	auto dot = FVector::DotProduct(unityPrimaryAxis, unitySecondaryAxis);
	if (!FMath::IsNearlyZero(dot)){
		UE_LOG(JsonLog, Warning, TEXT("Joint axes not perpendicular, errorneous behavior possible. Primary axis: %f %f %f, secondary axis: %f %f %f, dotPRoduc: %f"),
			unityPrimaryAxis.X, unityPrimaryAxis.Y, unityPrimaryAxis.Z, unitySecondaryAxis.X, unitySecondaryAxis.Y, unitySecondaryAxis.Z, dot);
	}
	unitySecondaryAxis = makePerpendicular(unityPrimaryAxis, unitySecondaryAxis);

	auto jointPos = jointObject.unityLocalPosToUnrealWorld(joint.anchor);

	auto jointX = unityPrimaryAxis;
	auto jointZ = makePerpendicular(jointX, unitySecondaryAxis);
	auto jointY = FVector::CrossProduct(jointX, jointZ);
	jointY.Normalize();

	FMatrix jointMatrix= FMatrix::Identity;
	jointMatrix.SetAxes(&jointX, &jointY, &jointZ, &jointPos);

	jointTransform.SetFromMatrix(jointMatrix);

	auto mainAxisLimit = angleRangeFromLimits(confData.lowAngularXLimit.limit, confData.highAngularXLimit.limit);
	//Unreal xAxis is Twist.
	physConstraint->SetAngularTwistLimit(unityAngularX, mainAxisLimit.range*0.5f);

	physConstraint->SetAngularSwing1Limit(unityAngularY, confData.angularYLimit.limit);

	physConstraint->SetAngularSwing2Limit(unityAngularZ, confData.angularZLimit.limit);

	//physConstraint->SetAngularSwing1Limit(unityAngularY, confData.angularYLimit.limit);

	//unreal y axis (xz plane)
	//physConstraint->SetAngularSwing2Limit(unityAngularX, mainAxisLimit.range * 0.5f);

	//unreal x axis (yz plane)
	//physConstraint->SetAngularTwistLimit(unityAngularZ, confData.angularYLimit.limit);

	//TODO: As of 2018.3, anglar drive on configurable joints is broken for me in inspector.

	//What the hell? Just one limit for all 3 axes? On unity side.	
	physConstraint->SetLinearXLimit(unityLinearX, unityDistanceToUe(confData.linearLimit.limit));
	physConstraint->SetLinearYLimit(unityLinearZ, unityDistanceToUe(confData.linearLimit.limit));
	physConstraint->SetLinearZLimit(unityLinearY, unityDistanceToUe(confData.linearLimit.limit));
}


void JointBuilder::setupSpringJointConstraint(UPhysicsConstraintComponent *physConstraint, int jointIndex, FTransform &jointTransform, 
const JsonPhysicsJoint &joint, const JsonGameObject &jointObject) const{
	using namespace UnrealUtilities;
	check(physConstraint);
	physConstraint->SetLinearXLimit(LCM_Free, 1.0f);
	physConstraint->SetLinearYLimit(LCM_Free, 1.0f);
	physConstraint->SetLinearZLimit(LCM_Free, 1.0f);
	physConstraint->SetAngularSwing1Limit(ACM_Free, 0.0f);
	physConstraint->SetAngularSwing2Limit(ACM_Free, 0.0f);
	physConstraint->SetAngularTwistLimit(ACM_Free, 0.0f);

	if (joint.springJointData.Num() == 0){
		UE_LOG(JsonLog, Warning, TEXT("Spring joint data not found on joint %d, obj %d (%s)"), jointIndex, jointObject.id, *jointObject.name);
		return;
	}
	const auto& springData = joint.springJointData[0];

	physConstraint->SetLinearPositionDrive(true, true, true);
	physConstraint->SetLinearPositionTarget(FVector::ZeroVector);
	physConstraint->SetLinearDriveParams(unityLinearSpringForceToUnreal(springData.spring), 0.0f, 0.0f);

	if ((springData.minDistance > 0.0f) || (springData.maxDistance > 0.0f)){
		UE_LOG(JsonLog, Warning, TEXT("Spring limits found on joint %d, obj %d (%s). Min distance %f, max distance %f. Spring limits are not supported"), jointIndex, jointObject.id, *jointObject.name,
			springData.minDistance, springData.maxDistance);
	}
}

void JointBuilder::setupCharacterJointConstraint(UPhysicsConstraintComponent *physConstraint, int jointIndex, FTransform &jointTransform,
	const JsonPhysicsJoint &joint, const JsonGameObject &jointObject) const{

	using namespace UnrealUtilities;
	check(physConstraint);
	/*
		Swing1 is XY plane rotation
		Swing2 is XZ plane.
		Twist is YZ.

		So, Z is swing1
		Y is swing2
		X is twist
	*/

	physConstraint->SetLinearXLimit(LCM_Locked, 1.0f);
	physConstraint->SetLinearYLimit(LCM_Locked, 1.0f);
	physConstraint->SetLinearZLimit(LCM_Locked, 1.0f);
	physConstraint->SetAngularSwing1Limit(ACM_Free, 0.0f);
	physConstraint->SetAngularSwing2Limit(ACM_Free, 0.0f);
	physConstraint->SetAngularTwistLimit(ACM_Free, 0.0f);

	if (joint.characterJointData.Num() == 0){
		UE_LOG(JsonLog, Warning, TEXT("Character joint data not found on joint %d, obj %d (%s)"), jointIndex, jointObject.id, *jointObject.name);
		return;
	}
	const auto& charData = joint.characterJointData[0];

	UE_LOG(JsonLog, Warning, TEXT("Joint axes axis: (%f; %f; %f), swing axis: (%f; %f; %f)"),
		joint.axis.X, joint.axis.Y, joint.axis.Z, charData.swingAxis.X, charData.swingAxis.Y, charData.swingAxis.Z);

	auto twistAxis = jointObject.unityLocalVectorToUnrealWorld(joint.axis);//z axis
	twistAxis.Normalize();
	auto swingAxis = jointObject.unityLocalVectorToUnrealWorld(charData.swingAxis);//...x axis? No, y axis, with x pointing forward
	swingAxis.Normalize();

	auto jointPos = jointObject.unityLocalPosToUnrealWorld(joint.anchor);

	auto dot = FVector::DotProduct(twistAxis, swingAxis);
	if (!FMath::IsNearlyZero(dot)){
		UE_LOG(JsonLog, Warning, TEXT("Joint axes not perpendicular on character %d, obj %d (%s). Swing axis: (%f; %f; %f), twist Axis: (%f; %f; %f), dot: %f"), 
			jointIndex, jointObject.id, *jointObject.name, swingAxis.X, swingAxis.Y, swingAxis.Z, twistAxis.X, twistAxis.Y, twistAxis.Z, dot);
	}

	auto jointX = twistAxis;
	auto jointZ = makePerpendicular(jointX, swingAxis);
	auto jointY = FVector::CrossProduct(jointX, jointZ);
	jointY.Normalize();

	FMatrix hingeMatrix = FMatrix::Identity;//Goddamit, again. Partially initialized matrix.
	hingeMatrix.SetAxes(&jointX, &jointY, &jointZ, &jointPos);

	jointTransform.SetFromMatrix(hingeMatrix);

	//swing 1
	if (FMath::IsNearlyZero(charData.swing1Limit.limit)){
		physConstraint->SetAngularSwing1Limit(ACM_Locked, 0.0f);
	}
	else{
		physConstraint->SetAngularSwing1Limit(ACM_Limited, charData.swing1Limit.limit);
	}

	//swing 2
	if (FMath::IsNearlyZero(charData.swing2Limit.limit)){
		physConstraint->SetAngularSwing2Limit(ACM_Locked, 0.0f);
	}
	else{
		physConstraint->SetAngularSwing2Limit(ACM_Limited, charData.swing2Limit.limit);
	}

	//twist
	float twistRange = charData.highTwistLimit.limit - charData.lowTwistLimit.limit;
	float twistMidAngle = (charData.highTwistLimit.limit + charData.lowTwistLimit.limit)* 0.5f;
	UE_LOG(JsonLog, Log, TEXT("high twist limit: %f, low twist limit: %f, Twist range: %f; twist mid angle : %f"), 
		charData.highTwistLimit.limit, charData.lowTwistLimit.limit, twistRange, twistMidAngle);

	if (FMath::IsNearlyZero(twistRange)){
		physConstraint->SetAngularTwistLimit(ACM_Locked, 0.0f);
	}
	else{
		physConstraint->SetAngularTwistLimit(ACM_Limited, twistRange*0.5f);
	}

	if (!FMath::IsNearlyZero(twistMidAngle)){
		auto constraintOffsetRotator = FRotator(0.0f, 0.0f, -twistMidAngle);
		physConstraint->ConstraintInstance.AngularRotationOffset += constraintOffsetRotator;
	}
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
	check(physConstraint);

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
		else if (curJoint.isSpringJointType()){
			setupSpringJointConstraint(physConstraint, jointIndex, jointTransform, curJoint, obj);
		}
		else if (curJoint.isCharacterJointType()){
			setupCharacterJointConstraint(physConstraint, jointIndex, jointTransform, curJoint, obj);
		}
		else if (curJoint.isConfigurableJointType()){
			setupConfigurableJointConstraint(physConstraint, jointIndex, jointTransform, curJoint, obj);
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

bool JointBuilder::getConstraintMotion(EAngularConstraintMotion &angMotion, const FString &arg){
	angMotion = ACM_Locked;
	if (arg == "Free"){
		angMotion = ACM_Free;
	}
	else if (arg == "Limited"){
		angMotion = ACM_Limited;
	}
	else if (arg == "Locked"){
		angMotion = ACM_Locked;
	}
	else
		return false;
	return true;
}

bool JointBuilder::getConstraintMotion(ELinearConstraintMotion &linearMotion, const FString &arg){
	linearMotion = LCM_Locked;
	if (arg == "Free"){
		linearMotion = LCM_Free;
	}
	else if (arg == "Limited"){
		linearMotion = LCM_Limited;
	}
	else if (arg == "Locked"){
		linearMotion = LCM_Locked;
	}
	else
		return false;
	return true;
}

EAngularConstraintMotion JointBuilder::getAngularMotionChecked(const FString &arg, const FString &motionName, int jointIndex, const JsonGameObject &jsonObj){
	EAngularConstraintMotion result = ACM_Locked;
	if (!getConstraintMotion(result, arg))
		UE_LOG(JsonLog, Warning, TEXT("Invalid angular motion \"%s\": \"%s\" on jsonObj %d(\"%s\"), joint index: %d"), 
			*motionName, *arg, jsonObj.id, *jsonObj.name, jointIndex);
	return result;
}

ELinearConstraintMotion JointBuilder::getLinearMotionChecked(const FString &arg, const FString &motionName, int jointIndex, const JsonGameObject &jsonObj){
	ELinearConstraintMotion result = LCM_Locked;
	if (!getConstraintMotion(result, arg))
		UE_LOG(JsonLog, Warning, TEXT("Invalid linear motion \"%s\": \"%s\" on jsonObj %d(\"%s\"), joint index: %d"), 
			*motionName, *arg, jsonObj.id, *jsonObj.name, jointIndex);

	return result;
}

void JointBuilder::AngularAdjustment::setMinMax(float min, float max){
	range = max - min;
	offset = (max + min) * 0.5f;
}

void JointBuilder::AngularAdjustment::setMin(float newMin){
	setMinMax(newMin, getMax());
}

void JointBuilder::AngularAdjustment::setMax(float newMax){
	setMinMax(getMin(), newMax);
}

float JointBuilder::AngularAdjustment::getMin() const{
	return offset - range * 0.5f;
}

float JointBuilder::AngularAdjustment::getMax() const{
	return offset + range * 0.5f;
}

JointBuilder::AngularAdjustment JointBuilder::angleRangeFromLimits(float min, float max){
	AngularAdjustment result;
	result.setMinMax(min, max);
	return result;
}
