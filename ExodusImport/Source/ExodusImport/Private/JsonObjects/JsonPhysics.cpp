#include "JsonImportPrivatePCH.h"
#include "JsonPhysics.h"
#include "macros.h"
#include "loggers.h"

void JsonObjectReference::load(JsonObjPtr data){
	using namespace JsonObjects;
	JSON_GET_VAR(data, instanceId);
	JSON_GET_VAR(data, isNull);
}

void JsonSoftJointLimit::load(JsonObjPtr data){
	using namespace JsonObjects;
	JSON_GET_VAR(data, limit);
	JSON_GET_VAR(data, spring);
}

void JsonSoftJointLimitSpring::load(JsonObjPtr data){
	using namespace JsonObjects;
	JSON_GET_VAR(data, spring);
	JSON_GET_VAR(data, damper);
}

void JsonJointMotor::load(JsonObjPtr data){
	using namespace JsonObjects;
	JSON_GET_VAR(data, targetVelocity);
	JSON_GET_VAR(data, force);
	JSON_GET_VAR(data, freeSpin);
}

void JsonJointDrive::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, positionSpring);
	JSON_GET_VAR(data, positionDamper);
	JSON_GET_VAR(data, maximumForce);
}

void JsonJointLimits::load(JsonObjPtr data){
	using namespace JsonObjects;
	JSON_GET_VAR(data, min);
	JSON_GET_VAR(data, max);
	JSON_GET_VAR(data, bounciness);
	JSON_GET_VAR(data, bounceMinVelocity);
	JSON_GET_VAR(data, contactDistance);
}

void JsonJointSpring::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, spring);
	JSON_GET_VAR(data, damper);
	JSON_GET_VAR(data, targetPosition);
}

void JsonHingeJointData::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, angle);
	getJsonObj(data, limits, "limits");
	getJsonObj(data, motor, "motor");
	getJsonObj(data, spring, "spring");
	JSON_GET_VAR(data, useLimits);
	JSON_GET_VAR(data, useMotor);
	JSON_GET_VAR(data, useSpring);
}

void JsonSpringJointData::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, damper);
	JSON_GET_VAR(data, maxDistance);
	JSON_GET_VAR(data, minDistance);
	JSON_GET_VAR(data, spring);
	JSON_GET_VAR(data, tolerance);
}

void JsonCharacterJointData::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, enableProjection);
	JSON_GET_VAR(data, highTwistLimit);
	JSON_GET_VAR(data, lowTwistLimit);
	JSON_GET_VAR(data, projectionAngle);
	JSON_GET_VAR(data, projectionDistance);

	JSON_GET_OBJ(data, swing1Limit);
	JSON_GET_OBJ(data, swing2Limit);
	JSON_GET_VAR(data, swingAxis);
	JSON_GET_OBJ(data, swingLimitSpring);
	JSON_GET_OBJ(data, twistLimitSpring);
}

void JsonConfigurableJointData::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_OBJ(data, angularXDrive);
	JSON_GET_OBJ(data, angularXLimitSpring);
	JSON_GET_VAR(data, angularXMotion);

	JSON_GET_OBJ(data, angularYLimit);
	JSON_GET_VAR(data, angularYMotion);

	JSON_GET_OBJ(data, angularYZDrive);
	JSON_GET_OBJ(data, angularYZLimitSpring);

	JSON_GET_OBJ(data, angularZLimit);
	JSON_GET_VAR(data, angularZMotion);

	JSON_GET_VAR(data, configuredInWorldSpace);
	JSON_GET_OBJ(data, highAngularXLimit);

	JSON_GET_OBJ(data, linearLimit);
	JSON_GET_OBJ(data, linearLimitSpring);

	JSON_GET_OBJ(data, lowAngularXLimit);
	JSON_GET_VAR(data, projectionAngle);
	JSON_GET_VAR(data, projectionDistance);
	JSON_GET_VAR(data, projectionMode);

	JSON_GET_VAR(data, rotationDriveMode);
	JSON_GET_VAR(data, secondaryAxis);

	JSON_GET_OBJ(data, slerpDrive);
	JSON_GET_VAR(data, swapBodies);

	JSON_GET_VAR(data, targetAngularVelocity);

	JSON_GET_VAR(data, targetPosition);
	JSON_GET_VAR(data, targetRotation);
	JSON_GET_VAR(data, targetVelocity);

	JSON_GET_OBJ(data, xDrive);
	JSON_GET_VAR(data, xMotion);

	JSON_GET_OBJ(data, yDrive);
	JSON_GET_VAR(data, yMotion);

	JSON_GET_OBJ(data, zDrive);
	JSON_GET_VAR(data, zMotion);
}

void JsonPhysicsJoint::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, jointType);

	JSON_GET_VAR(data, anchor);
	JSON_GET_VAR(data, connectedAnchor);
	JSON_GET_OBJ(data, connectedBody);
	JSON_GET_OBJ(data, connectedBodyObject);
	JSON_GET_VAR(data, axis);
	JSON_GET_VAR(data, breakForce);
	JSON_GET_VAR(data, breakTorque);

	JSON_GET_VAR(data, connectedMassScale);
	JSON_GET_VAR(data, enableCollision);
	JSON_GET_VAR(data, autoConfigureConnectedAnchor);

	JSON_GET_ARRAY(data, sprintJointData);
	JSON_GET_ARRAY(data, hingeJointData);
	JSON_GET_ARRAY(data, configurableJointData);
	JSON_GET_ARRAY(data, springJointData);
	JSON_GET_ARRAY(data, characterJointData);
}
