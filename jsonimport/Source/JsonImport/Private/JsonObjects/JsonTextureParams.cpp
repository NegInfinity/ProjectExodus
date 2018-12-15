#include "JsonImportPrivatePCH.h"
#include "JsonTextureParams.h"
#include "macros.h"

#define JSON_ENABLE_VALUE_LOGGING

void JsonTextureParams::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, imageHash);
	JSON_GET_VAR(data, initialized);
	JSON_GET_VAR(data, anisoLevel);
	JSON_GET_VAR(data, dimension);
	JSON_GET_VAR(data, filterMode);
	JSON_GET_VAR(data, width);
	JSON_GET_VAR(data, height);
	JSON_GET_VAR(data, mipMapBias);
	JSON_GET_VAR(data, texelSize);
	JSON_GET_VAR(data, wrapMode);
	JSON_GET_VAR(data, wrapModeU);
	JSON_GET_VAR(data, wrapModeV);
	JSON_GET_VAR(data, wrapModeW);
}

