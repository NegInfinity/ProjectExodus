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

	int customCubemapId = -1;
	int customTex2dId = -1;

	void load(JsonObjPtr jsonData);
	JsonReflectionProbe() = default;
	JsonReflectionProbe(JsonObjPtr jsonData);
};

