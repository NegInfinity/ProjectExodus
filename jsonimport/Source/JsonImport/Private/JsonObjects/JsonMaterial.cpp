#include "JsonImportPrivatePCH.h"
#include "JsonMaterial.h"
#include "macros.h"
#include "utilities.h"

//#define JSON_ENABLE_VALUE_LOGGING

using namespace JsonObjects;

void JsonMaterial::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR_LOG(data, renderQueue);
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, shader);

	JSON_GET_VAR_LOG(data, blendMode);

	JSON_GET_VAR(data, supportedShader);

	JSON_GET_VAR(data, mainTexture);
	JSON_GET_VAR(data, mainTextureOffset);
	JSON_GET_VAR(data, mainTextureScale);
	JSON_GET_PARAM(data, color, getColor);

	/*
	I think those flags are quite glitchy at best, as they're derived from shader keywords. 
	I need to rethink the way the shader reconstruction operates
	*/
	JSON_GET_VAR(data, useNormalMap);
	JSON_GET_VAR(data, useAlphaTest);
	JSON_GET_VAR(data, useAlphaBlend);
	JSON_GET_VAR(data, useAlphaPremultiply);
	JSON_GET_VAR(data, useEmission);
	JSON_GET_VAR(data, useParallax);
	JSON_GET_VAR(data, useDetailMap);
	JSON_GET_VAR(data, useMetallic);
			
	JSON_GET_VAR(data, hasMetallic);
	JSON_GET_VAR(data, hasSpecular);
	JSON_GET_VAR(data, hasEmissionColor);
	JSON_GET_VAR(data, hasEmission);
			
	JSON_GET_VAR(data, useSpecular);

	JSON_GET_VAR(data, albedoTex);
	JSON_GET_VAR(data, specularTex);
	JSON_GET_VAR(data, metallicTex);
	JSON_GET_VAR(data, normalMapTex);
	JSON_GET_VAR(data, occlusionTex);
	JSON_GET_VAR(data, parallaxTex);
	JSON_GET_VAR(data, emissionTex);
	JSON_GET_VAR(data, detailMaskTex);
	JSON_GET_VAR(data, detailAlbedoTex);
	JSON_GET_VAR(data, detailAlbedoOffset);
	JSON_GET_VAR(data, detailAlbedoScale);
	JSON_GET_VAR(data, detailNormalMapScale);

	JSON_GET_VAR(data, detailNormalMapTex);

	JSON_GET_VAR(data, alphaCutoff);
	JSON_GET_VAR(data, smoothness);
	JSON_GET_VAR(data, smoothnessScale);
	JSON_GET_VAR(data, specularColor);
	JSON_GET_VAR(data, metallic);
	JSON_GET_VAR(data, bumpScale);
	JSON_GET_VAR(data, parallaxScale);
	JSON_GET_VAR(data, occlusionStrength);
	JSON_GET_VAR(data, emissionColor);
	JSON_GET_VAR(data, detailMapScale);
	JSON_GET_VAR(data, secondaryUv);

	JSON_GET_VAR(data, smoothnessMapChannel);
	JSON_GET_VAR(data, specularHighlights);
	JSON_GET_VAR(data, glossyReflections);

	colorGammaCorrected = applyGamma(color);
	specularColorGammaCorrected = applyGamma(specularColor);
	emissionColorGammaCorrected = applyGamma(emissionColor);
}

JsonMaterial::JsonMaterial(JsonObjPtr data){
	load(data);
}

static bool checkTextureOffset(const FVector2D& offset, const FVector2D& scale){
	return (offset != FVector2D(0.0f, 0.0f)) ||
		(scale != FVector2D(1.0f, 1.0f));
}

bool JsonMaterial::usesMainTextureTransform() const{
	return checkTextureOffset(mainTextureOffset, mainTextureScale);
}

bool JsonMaterial::usesDetailTextureTransform() const{
	return checkTextureOffset(detailAlbedoOffset, detailAlbedoScale);
}

bool JsonMaterial::isTransparentQueue() const{
	//return (renderQueue >= 3000) && (renderQueue < 4000);
	return (renderQueue >= Queues::Transparent) && (renderQueue < Queues::Overlay);
}

bool JsonMaterial::isAlphaTestQueue() const{
	return (renderQueue >= Queues::AlphaTest) && (renderQueue < Queues::Transparent);
	//return (renderQueue >= 2450) && (renderQueue < 3000);
}

bool JsonMaterial::isGeomQueue() const{
	return (!isTransparentQueue() && !isAlphaTestQueue()) 
		|| ((renderQueue >= Queues::Geometry) && (renderQueue < Queues::AlphaTest));
	/*
	return (!isTransparentQueue() && !isAlphaTestQueue()) 
		|| ((renderQueue >= 2000) && (renderQueue < 2450));*/
}

bool JsonMaterial::isTransparentMode() const{
	return blendMode == BlendModes::Transparent;
}

bool JsonMaterial::isAlphaTestMode() const{
	return blendMode == BlendModes::Cutout;
}

bool JsonMaterial::isFadeMode() const{
	return blendMode == BlendModes::Fade;//Bah. So much for the scoped enums.
}

bool JsonMaterial::nameMarkedTransparent() const{
	return name.Contains(TEXT("/Transparent/"), ESearchCase::CaseSensitive);
}

bool JsonMaterial::nameMarkedCutout() const{
	return name.Contains(TEXT("/Transparent/Cutout/"), ESearchCase::CaseSensitive);
}

FString JsonMaterial::getUnrealMaterialName() const{
	//Duplicated code. Need to do something about it later.
	auto pathBaseName = FPaths::GetBaseFilename(path);
	FString result;
	if (!pathBaseName.IsEmpty()){
		//Well, I've managed to create a level with two meshes named "cube". So...
		result = FString::Printf(TEXT("%s_%s_%d"), *pathBaseName, *name, id);
	}
	else{
		result = FString::Printf(TEXT("%s_%d"), *name, id);
	}
	return result;
}
