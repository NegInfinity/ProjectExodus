#pragma once
#include "JsonTypes.h"

class JsonLight{
public:
	float range;
	float spotAngle;
	FString lightType;
	float shadowStrength;
	float intensity;
	float bounceIntensity;
	FLinearColor color;
	FString renderMode;
	FString shadows;
	bool castsShadows;

	void load(JsonObjPtr jsonData);
	JsonLight() = default;
	JsonLight(JsonObjPtr jsonData);
};

