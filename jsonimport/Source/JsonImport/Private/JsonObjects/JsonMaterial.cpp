#include "JsonImportPrivatePCH.h"
#include "JsonMaterial.h"
#include "macros.h"

using namespace JsonObjects;

void JsonMaterial::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, renderQueue);
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, shader);

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
	JSON_GET_VAR(data, detailNormalMapTex);

	JSON_GET_VAR(data, alphaCutoff);
	JSON_GET_VAR(data, smoothness);
	JSON_GET_VAR(data, specularColor);
	JSON_GET_VAR(data, metallic);
	JSON_GET_VAR(data, bumpScale);
	JSON_GET_VAR(data, parallaxScale);
	JSON_GET_VAR(data, occlusionStrength);
	JSON_GET_VAR(data, emissionColor);
	JSON_GET_VAR(data, detailMapScale);
	JSON_GET_VAR(data, secondaryUv);
}

JsonMaterial::JsonMaterial(JsonObjPtr data){
	load(data);
}
