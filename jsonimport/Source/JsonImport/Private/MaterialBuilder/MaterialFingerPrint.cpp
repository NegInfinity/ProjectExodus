#include "JsonImportPrivatePCH.h"
#include "MaterialFingerprint.h"
#include "JsonObjects.h"

using namespace JsonObjects;

void MaterialFingerprint::clear(){
	id = 0;
}

MaterialFingerprint::MaterialFingerprint(const JsonMaterial& src){
	id = 0;
	albedoTex = isValidId(src.albedoTex);
	normalmapTex = isValidId(src.normalMapTex);
	metallicTex = isValidId(src.metallicTex);
	specularTex = isValidId(src.specularTex);
	occlusionTex = isValidId(src.occlusionTex);
	parallaxTex = isValidId(src.parallaxTex);
	detailMaskTex = isValidId(src.detailMaskTex);
	detailAlbedoTex = isValidId(src.detailAlbedoTex);
	detailNormalTex = isValidId(src.detailNormalMapTex);
	emissionTex = isValidId(src.emissionTex);

	//flags
	specularModel = src.hasSpecular;//yup, this overrules metallic
	emissionEnabled = src.hasEmission;//
	mainTextureTransform = src.usesMainTextureTransform();
	detailTextureTransform = src.usesDetailTextureTransform();

	normalMapIntensity = src.bumpScale != 1.0f;
	occlusionIntensity = src.occlusionStrength != 1.0f;

	altSmoothnessTexture = src.smoothnessMapChannel != 0;

	secondaryUv = (int)src.secondaryUv;
}

MaterialFingerprint::MaterialFingerprint()
:id(0){
}

FString MaterialFingerprint::getMatName() const{
	FString result;
	result += TEXT("Mat");

	result += specularModel ? TEXT("Spec"): TEXT("Metal");
	if (emissionEnabled){
		result += TEXT("Emit");
		if (emissionTex)
			result += TEXT("Tex");
		result += TEXT("-");
	}
	if (mainTextureTransform){
		result += TEXT("MainTxTrsf-");
	}
	if (detailTextureTransform){
		result += TEXT("DetTxTrsf-");
	}
	if (albedoTex){
		result += TEXT("AlbTex-");
	}
	if (normalmapTex){
		result += TEXT("NrmTex");
		if (normalMapIntensity){
			result += TEXT("(var)");
		}
		result += TEXT("-");
	}
	if (metallicTex){
		result += TEXT("MtlTex - ");
	}
	if (specularTex){
		result += TEXT("SpcTex - ");
	}
	if (occlusionTex){
		result += TEXT("OclnTex");
		if (occlusionIntensity){
			result += TEXT("(var)");
		}
		result += TEXT("-");
	}
	if (parallaxTex){
		result += TEXT("Prlx-");
	}
	if (detailMaskTex){
		result += TEXT("DtlMask-");
	}
	if (detailAlbedoTex){
		result += TEXT("DtlAlb-");
	}
	if (detailAlbedoTex){
		result += TEXT("DtlNrm-");
	}
	result += FString::Printf(TEXT("Smth%d-"), altSmoothnessTexture);
	if (secondaryUv != 0)
		result += FString::Printf(TEXT("uv%d-"), secondaryUv);

	FString idStr = FString::Printf(TEXT("-id%08x"), id);
	result += idStr;

	return result;
}
