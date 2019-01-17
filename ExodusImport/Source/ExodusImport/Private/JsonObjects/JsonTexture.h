#pragma once

#include "JsonTypes.h"
#include "JsonTextureParams.h"
#include "JsonTextureImportParams.h"

class JsonTexture{
public:
	FString name;
	JsonId id = -1;
	FString path;
	FString filterMode;
	float mipMapBias;
	int32 width;
	int32 height;
	FString wrapMode;
	bool isTex2D;
	bool isRenderTarget;
	bool alphaTransparency;
	float anisoLevel;
	//FString base64;//??? Pretty sure this is no longer needed
	bool importDataFound;
	bool sRGB;
	FString textureType;// = "default";
	bool normalMapFlag = false;

	JsonTextureParams textureParams;
	JsonTextureImportParams textureImportParams;

	void load(JsonObjPtr data);
	JsonTexture() = default;
	JsonTexture(JsonObjPtr data){
		load(data);
	}
};
