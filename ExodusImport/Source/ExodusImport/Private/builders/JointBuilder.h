#pragma once
#include "JsonTypes.h"
#include "InstanceIdMap.h"
#include "ImportWorkData.h"

class UPhysicsConstraintComponent;

class JointBuilder{
protected:
	const JsonGameObject* resolveObjectReference(const JsonObjectReference &ref, const InstanceIdMap &instanceMap, const TArray<JsonGameObject>& objects) const;
	void processPhysicsJoint(const JsonGameObject &obj, const InstanceIdMap &instanceMap,
		const TArray<JsonGameObject>& objects, ImportWorkData &workData) const;
	static void buildInstanceIdMap(InstanceIdMap &outMap, const TArray<JsonGameObject>& objects);

	const bool isSupportedJoint(const JsonPhysicsJoint &joint) const;

	void setupFixedJointConstraint(UPhysicsConstraintComponent *physConstraint, const JsonPhysicsJoint &joint) const;
	void setupHingeJointConstraint(UPhysicsConstraintComponent *physComponent, int jointIndex, 
		FTransform &jointTransform, const JsonPhysicsJoint &joint, const JsonGameObject &jointObject) const;
	void setupSpringJointConstraint(UPhysicsConstraintComponent *physComponent, int jointIndex, FTransform &jointTransform, 
		const JsonPhysicsJoint &joint, const JsonGameObject &jointObject) const;
	void setupCharacterJointConstraint(UPhysicsConstraintComponent *physComponent, int jointIndex, FTransform &jointTransform,
		const JsonPhysicsJoint &joint, const JsonGameObject &jointObject) const;

	static float unityLinearSpringForceToUnreal(float force){
		return force;
	}
	static float unityTorqueToUnreal(float torque){
		return torque * 10000.0f;
	}
	static float unityAngularMotorForceToUnreal(float force){
		return force * 100.0f; 
	}
	static float unityAngularSpringForceToUnreal(float force){
		return force * 10.0f;//Why? *100.0f does not seem to produce correct results....
	}
	static float unityForceToUnreal(float force){
		return force * 100.0f;//It seems that forces are also expressed in somehow centimeter-related units.
	}


public:
	void processPhysicsJoints(const JsonGameObjectArray& objects, ImportWorkData &workData) const;
};
