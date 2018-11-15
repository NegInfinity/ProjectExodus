#pragma once

#include "JsonTypes.h"
#include "JsonTextureParams.h"
#include "JsonTextureImportParams.h"

class JsonCubemap{
public:
	int id;
	FString name;
	FString exportPath;
	FString assetPath;
	bool needConversion;

	JsonTextureParams texParams;
	JsonTextureImportParams texImportParams;

	void load(JsonObjPtr data);
	JsonCubemap() = default;
	JsonCubemap(JsonObjPtr data){
		load(data);
	}
};