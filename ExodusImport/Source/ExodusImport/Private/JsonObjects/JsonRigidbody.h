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

	bool usesInterpolation() const{
		return interpolation == "Interpolate";
	}

	bool usesExtrapolation() const{
		return interpolation == "Extrapolate";
	}

	bool usesDiscreteCollision() const{
		return collisionDetectionMode == "Discrete";
	}

	bool usesContinuousCollision() const{
		return collisionDetectionMode == "Continuous";
	}

	bool usesContinuousDynamicCollision() const{
		return collisionDetectionMode == "ContinuousDynamic";
	}

	bool usesContinuousSpeculativeCollision() const{
		return collisionDetectionMode == "ContinuousSpeculative";
	}

	JsonRigidbody() = default;
	JsonRigidbody(JsonObjPtr data);
	void load(JsonObjPtr data);
};