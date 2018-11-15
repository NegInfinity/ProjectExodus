#pragma once

#include "JsonTypes.h"

class JsonTextureParams{
public:
	FString name;
	FString imageHash;
	bool initialized = false;
	int anisoLevel = 0;
	FString dimension;
	FString filterMode;
	int width = 0;
	int height = 0;
	float mipMapBias = 0.0f;
	FVector2D texelSize = FVector2D::ZeroVector;
	FString wrapMode;
	FString wrapModeU;
	FString wrapModeV;
	FString wrapModeW;

	void load(JsonObjPtr data);
	JsonTextureParams() = default;
	JsonTextureParams(JsonObjPtr data){
		load(data);
	}
};