#include "JsonImportPrivatePCH.h"
#include "JsonTextureImportParams.h"
#include "macros.h"

#define JSON_ENABLE_VALUE_LOGGING

using namespace JsonObjects;

void JsonTextureImportParams::load(JsonObjPtr data){
	JSON_GET_VAR(data, initialized);
	JSON_GET_VAR(data, assetPath);
	JSON_GET_VAR(data, borderMipmap);
	JSON_GET_VAR(data, compressionQuality);
	JSON_GET_VAR(data, convertToNormalmap);
	JSON_GET_VAR(data, crunchedCompression);
	JSON_GET_VAR(data, fadeout);
	JSON_GET_VAR(data, filterMode);
	JSON_GET_VAR(data, generateCubemap);
	JSON_GET_VAR(data, isReadable);
	JSON_GET_VAR(data, maxTextureSize);
	JSON_GET_VAR(data, mipmapFadeDistanceEnd);
	JSON_GET_VAR(data, mipmapFadeDistanceStart);
	JSON_GET_VAR2(data, mipmapsPreserveCoverage, mipMapsPreserveCoverage);
	JSON_GET_VAR(data, heightmapScale);
	JSON_GET_VAR(data, npotScale);
	JSON_GET_VAR(data, qualifiesForSpritePacking);
	JSON_GET_VAR(data, spriteBorder);
	JSON_GET_VAR(data, spriteImportMode);
	JSON_GET_VAR(data, spritePackingTag);
	JSON_GET_VAR(data, spritePivot);
	JSON_GET_VAR(data, spritePixelsPerUnit);

	getJsonObjArray(data, spritesheet, "spritesheet");
	//TArray<JsonSpriteMetaData> spritesheet;

	JSON_GET_VAR(data, sRGBTexture);
	JSON_GET_VAR(data, streamingMipmaps);
	JSON_GET_VAR(data, streamingMipmapsPriority);
	JSON_GET_VAR(data, textureCompression);
	JSON_GET_VAR(data, textureType);
	JSON_GET_VAR(data, textureShape);
	JSON_GET_VAR(data, userData);
	JSON_GET_VAR(data, wrapMode);
	JSON_GET_VAR(data, wrapModeU);
	JSON_GET_VAR(data, wrapModeV);
	JSON_GET_VAR(data, wrapModeW);
}
