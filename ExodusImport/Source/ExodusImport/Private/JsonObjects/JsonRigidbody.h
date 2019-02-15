#pragma once
#include "JsonTypes.h"

class JsonRigidbody{
public:
	float angularDrag = 0.0f;
	FVector angularVelocity;
	FVector centerOfMass;

	FString collisionDetectionMode;
	int constraints = 0;
	bool detectCollisions = false;
	bool drag = false;

	bool freezeRotation = false;
	FVector inertiaTensor;
	FString interpolation;

	bool isKinematic = false;
	float mass = 1.0f;

	float maxAngularVelocity;
	float maxDepenetrationVelocity;

	FVector position;
	FVector4 rotation;

	float sleepThreshold;
	int solverIterations;
	int solverVelocityIterations;

	bool useGravity = true;
	FVector velocity = FVector::ZeroVector;
	FVector worldCenterOfMass = FVector::ZeroVector;

	JsonRigidbody() = default;
	JsonRigidbody(JsonObjPtr data);
	void load(JsonObjPtr data);
};