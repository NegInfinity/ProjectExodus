#include "JsonImportPrivatePCH.h"
#include "JsonRigidbody.h"
#include "macros.h"

using namespace JsonObjects;

JsonRigidbody::JsonRigidbody(JsonObjPtr data){
	load(data);
}

void JsonRigidbody::load(JsonObjPtr data){
	/*I should probably get rid of those helper macros due to reduced error checking...*/
	JSON_GET_VAR(data, angularDrag);
	JSON_GET_VAR(data, angularVelocity);
	JSON_GET_VAR(data, centerOfMass);

	JSON_GET_VAR(data, collisionDetectionMode);
	JSON_GET_VAR(data, constraints);
	JSON_GET_VAR(data, detectCollisions);
	JSON_GET_VAR(data, drag);

	JSON_GET_VAR(data, freezeRotation);
	JSON_GET_VAR(data, inertiaTensor);
	JSON_GET_VAR(data, interpolation);

	JSON_GET_VAR(data, isKinematic);
	JSON_GET_VAR(data, mass);

	JSON_GET_VAR(data, maxAngularVelocity);
	JSON_GET_VAR(data, maxDepenetrationVelocity);

	JSON_GET_VAR(data, position);
	JSON_GET_VAR(data, rotation);

	JSON_GET_VAR(data, sleepThreshold);
	JSON_GET_VAR(data, solverIterations);
	JSON_GET_VAR(data, solverVelocityIterations);

	JSON_GET_VAR(data, useGravity);
	JSON_GET_VAR(data, velocity);
	JSON_GET_VAR(data, worldCenterOfMass);
}
