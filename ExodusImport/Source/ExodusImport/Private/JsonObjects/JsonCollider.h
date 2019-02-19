#pragma once
#include "JsonTypes.h"
#include "JsonBounds.h"

class JsonCollider{
public:
	enum ColliderDirection{
		XAxis = 0, YAxis = 1, ZAxis = 2
	};
	FString colliderType;
	FVector center = FVector::ZeroVector;
	FVector size = FVector::ZeroVector;
	int direction = 0;//0 == x, 1 == y, 2 == z
	float radius = 0.0f;//Used for both sphere and capsule caps
	float height = 0.0f;//for capsules only
	int meshId = -1;//for meshes only

	bool  enabled = true;
	float contactOffset = 0.0f;
	JsonBounds bounds;
	bool trigger = false;

	JsonCollider() = default;
	JsonCollider(JsonObjPtr data);
	void load(JsonObjPtr data);
};