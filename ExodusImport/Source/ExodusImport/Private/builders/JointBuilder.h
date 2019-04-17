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

public:
	void processPhysicsJoints(const JsonGameObjectArray& objects, ImportWorkData &workData) const;
};
