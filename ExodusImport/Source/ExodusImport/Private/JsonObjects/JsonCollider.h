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
	float radius = 0.0f;
	float height = 0.0f;
	int meshId = -1;

	bool  enabled = true;
	float contactOffset = 0.0f;
	JsonBounds bounds;
	bool trigger = false;

	JsonCollider() = default;
	JsonCollider(JsonObjPtr data);
	void load(JsonObjPtr data);
};