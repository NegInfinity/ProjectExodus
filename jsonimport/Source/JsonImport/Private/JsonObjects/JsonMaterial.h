#pragma once
#include "JsonTypes.h"

class JsonMaterial{
public:
	int id = -1;
	int renderQueue = 0;
	FString name;
	FString path;
	FString shader;

	int mainTexture = -1;
	FVector2D mainTextureOffset = FVector2D(0.0f, 0.0f);//Vector2.zero;
	FVector2D mainTextureScale = FVector2D(1.0f, 1.0f);//Vector2.one;
	FLinearColor color = FLinearColor::White;// = Color.white;

	/*
	I think those flags are quite glitchy at best, as they're derived from shader keywords. 
	I need to rethink the way the shader reconstruction operates
	*/
	bool useNormalMap = false;
	bool useAlphaTest = false;
	bool useAlphaBlend = false;
	bool useAlphaPremultiply = false;
	bool useEmission = false;
	bool useParallax = false;
	bool useDetailMap = false;
	bool useMetallic = false;//this is unrealiable
			
	bool hasMetallic = false;
	bool hasSpecular = false;
	bool hasEmissionColor = false;
	bool hasEmission = false;
			
	bool useSpecular = false;

	int albedoTex = -1;
	int specularTex = -1;
	int metallicTex = -1;
	int normalMapTex = -1;
	int occlusionTex = -1;
	int parallaxTex = -1;
	int emissionTex = -1;
	int detailMaskTex = -1;
	int detailAlbedoTex = -1;
	int detailNormalMapTex = -1;
	float alphaCutoff = 1.0f;
	float smoothness = 0.5f;
	FLinearColor specularColor = FLinearColor::White;//Color.white;
	float metallic = 0.5f;
	float bumpScale = 1.0f;
	float parallaxScale = 1.0f;
	float occlusionStrength = 1.0f;
	FLinearColor emissionColor = FLinearColor::Black;//Color.black;
	float detailMapScale = 1.0f;
	float secondaryUv = 1.0f;

	JsonMaterial() = default;
	void load(JsonObjPtr data);
	JsonMaterial(JsonObjPtr data);
};