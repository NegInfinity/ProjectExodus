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
	FString rawPath;
	bool needConversion;
	FString format;
	bool isHdr = false;

	JsonTextureParams texParams;
	JsonTextureImportParams texImportParams;

	void load(JsonObjPtr data);
	JsonCubemap() = default;
	JsonCubemap(JsonObjPtr data){
		load(data);
	}
};