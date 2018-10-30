#pragma once
#include "JsonTypes.h"

class JsonSplatPrototype{
public:
	JsonTextureId textureId = -1;
	JsonTextureId normalMapId = -1;
			
	float metallic = 0.0f;
	float smoothness = 0.0f;
	FLinearColor specular = FLinearColor::White;
	FVector2D tileOffset = FVector2D(0.0f, 0.0f);
	FVector2D tileSize = FVector2D(1.0f, 1.0f);

	JsonSplatPrototype() = default;
	void load(JsonObjPtr data);
	JsonSplatPrototype(JsonObjPtr data);
};
