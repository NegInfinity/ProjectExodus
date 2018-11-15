#pragma once
#include "JsonTypes.h"
#include "JsonSpriteMetaData.h"

class JsonTextureImportParams{
public:
	bool initialized = false;
	FString assetPath;
	bool borderMipmap;
	int compressionQuality;
	bool convertToNormalmap;
	bool crunchedCompression;
	bool fadeout;
	FString filterMode;
	FString generateCubemap;
	bool isReadable;
	int maxTextureSize;
	int mipmapFadeDistanceEnd;
	int mipmapFadeDistanceStart;
	bool mipmapPreserveCoverage;
	float heightmapScale;
	FString npotScale;
	bool qualifiesForSpritePacking;
	FVector4 spriteBorder;
	FString spriteImportMode;
	FString spritePackingTag;
	FVector2D spritePivot;
	float spritePixelsPerUnit;

	TArray<JsonSpriteMetaData> spritesheet;

	bool sRGBTexture;
	bool streamingMipmaps;
	int streamingMipmapsPriority;
	FString textureCompression;
	FString textureType;
	FString textureShape;
	FString userData;
	FString wrapMode;
	FString wrapModeU;
	FString wrapModeV;
	FString wrapModeW;

	void load(JsonObjPtr data);
	JsonTextureImportParams() = default;
	JsonTextureImportParams(JsonObjPtr data){
		load(data);
	}
};