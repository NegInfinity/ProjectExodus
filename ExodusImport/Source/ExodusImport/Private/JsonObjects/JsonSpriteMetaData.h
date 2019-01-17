#pragma once

#include "JsonTypes.h"
#include "JsonRect.h"

class JsonSpriteMetaData{
public:
	FString name;
	FVector2D pivot;
	FVector4 border;
	int alignment;
	JsonRect rect;

	void load(JsonObjPtr jsonData);
	JsonSpriteMetaData() = default;
	JsonSpriteMetaData(JsonObjPtr jsonData){
		load(jsonData);
	}
};