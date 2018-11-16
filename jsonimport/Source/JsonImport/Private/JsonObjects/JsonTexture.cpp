#include "JsonImportPrivatePCH.h"
#include "JsonTexture.h"
#include "macros.h"

using namespace JsonObjects;

void JsonTexture::load(JsonObjPtr data){
	JSON_GET_VAR(data, name)
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, filterMode);
	JSON_GET_VAR(data, mipMapBias);
	JSON_GET_VAR(data, width);
	JSON_GET_VAR(data, height);
	JSON_GET_VAR(data, wrapMode);
	JSON_GET_VAR(data, isTex2D);
	JSON_GET_VAR(data, isRenderTarget);
	JSON_GET_VAR(data, alphaTransparency);
	JSON_GET_VAR(data, anisoLevel);
	JSON_GET_VAR(data, importDataFound);
	JSON_GET_VAR(data, sRGB);
	JSON_GET_VAR(data, textureType);
	JSON_GET_VAR(data, normalMapFlag);

	getJsonObj(data, textureParams, "texParams");
	//JsonTextureParams textureParams;
	//JsonTextureImportParams textureImportParams;
	getJsonObj(data, textureImportParams, "texImportParams");
}
