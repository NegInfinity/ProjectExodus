#pragma once
#include "JsonTypes.h"
#include "JsonBounds.h"

class JsonCollider{
public:
	enum ColliderDirection{
		XAxis = 0, YAxis = 1, ZAxis = 2
	};
	FString colliderType;
	int colliderIndex = -1;
	FVector center = FVector::ZeroVector;
	FVector size = FVector::ZeroVector;
	int direction = 0;//0 == x, 1 == y, 2 == z
	float radius = 0.0f;//Used for both sphere and capsule caps
	float height = 0.0f;//for capsules only
	//int meshId = -1;//for meshes only
	ResId meshId;

	bool  enabled = true;
	float contactOffset = 0.0f;
	JsonBounds bounds;
	bool trigger = false;

	bool isMeshCollider() const{
		return (colliderType == "mesh");
	}

	bool isSphereCollider() const{
		return (colliderType == "sphere");
	}

	bool isCapsuleCollider() const{
		return (colliderType == "capsule");
	}

	bool isBoxCollider() const{
		return (colliderType == "box");
	}

	JsonCollider() = default;
	JsonCollider(JsonObjPtr data);
	void load(JsonObjPtr data);
};