#pragma once
#include "JsonTypes.h"

class JsonReflectionProbe{
public:
	FLinearColor backgroundColor;
	float blendDistance;
	bool boxProjection;
	FVector center;
	FVector size;
	FString clearType;
	int32 cullingMask;
	bool hdr;
	float intensity;
	float nearClipPlane;
	float farClipPlane;
	int32 resolution;
	FString mode;
	FString refreshMode;

	void load(JsonObjPtr jsonData);
	JsonReflectionProbe() = default;
	JsonReflectionProbe(JsonObjPtr jsonData);
};

