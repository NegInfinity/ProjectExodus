#pragma once
#include "JsonTypes.h"

class JsonTreeInstance{
public:
	FColor color = FColor::White;
	float heightScale = 1.0f;
	FColor lightmapColor = FColor::White;
	FVector position = FVector::ZeroVector;
	float rotation = 0.0f;
	int32 prototypeIndex = 0;
	float widthScale = 1.0f;

	JsonTreeInstance() = default;
	JsonTreeInstance(JsonObjPtr data);
	void load(JsonObjPtr data);
};
