#include "JsonImportPrivatePCH.h"
#include "MaterialBuilder.h"

#include "JsonImporter.h"

#include "TerrainBuilder.h"

#include "JsonObjects/JsonTerrainDetailPrototype.h"

#include "MaterialTools.h"
#include "JsonObjects/utilities.h"
#include "UnrealUtilities.h"
#include "MaterialExpressionBuilder.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"

//#include "MaterialUtilities.h"

DEFINE_LOG_CATEGORY(JsonLogMatNodeSort);

using namespace MaterialTools;
using namespace UnrealUtilities;

UMaterial* MaterialBuilder::importMaterial(const JsonMaterial& jsonMat, JsonImporter *importer, JsonMaterialId matId){
	MaterialFingerprint fingerprint(jsonMat);

	FString sanitizedMatName;
	FString sanitizedPackageName;

	UMaterial *existingMaterial = nullptr;
	UPackage *matPackage = importer->createPackage(
		jsonMat.name, jsonMat.path, importer->getAssetRootPath(), FString("Material"), 
		&sanitizedPackageName, &sanitizedMatName, &existingMaterial);

	if (existingMaterial){
		importer->registerMaterialPath(jsonMat.id, existingMaterial->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing material: %s (package %s)"), *sanitizedMatName, *sanitizedPackageName);
		return existingMaterial;
	}

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPackage, FName(*sanitizedMatName), RF_Standalone|RF_Public,
		0, GWarn);

	//stuff
	MaterialBuildData buildData(matId, importer);
	buildMaterial(material, jsonMat, fingerprint, buildData);

	if (material){
		material->PreEditChange(0);
		material->PostEditChange();

		importer->registerMaterialPath(jsonMat.id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(material);
		matPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();

	return material;
}

UMaterial* MaterialBuilder::importMaterial(JsonObjPtr obj, JsonImporter *importer, JsonMaterialId matId){
	UE_LOG(JsonLog, Log, TEXT("Importing material %d"), matId);

	JsonMaterial jsonMat(obj);

	return importMaterial(jsonMat, importer, matId);
}

UMaterialInstanceConstant* MaterialBuilder::importMaterialInstance(JsonObjPtr obj, JsonImporter *importer, JsonMaterialId matId){
	UE_LOG(JsonLog, Log, TEXT("Importing material instance %d"), matId);

	JsonMaterial jsonMat(obj);

	return importMaterialInstance(jsonMat, importer, matId);
}


UMaterial* MaterialBuilder::createMaterial(const FString& name, const FString &path, JsonImporter *importer, 
		MaterialCallbackFunc newCallback, MaterialCallbackFunc existingCallback, MaterialCallbackFunc postEditCallback){
	FString sanitizedMatName;
	FString sanitizedPackageName;

	UMaterial *existingMaterial = nullptr;
	UPackage *matPackage = importer->createPackage(
		name, path, importer->getAssetRootPath(), FString("Material"), 
		&sanitizedPackageName, &sanitizedMatName, &existingMaterial);

	if (existingMaterial){
		if (existingCallback)
			existingCallback(existingMaterial);
		return existingMaterial;
	}

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPackage, FName(*sanitizedMatName), RF_Standalone|RF_Public,
		0, GWarn);

	if (newCallback)
		newCallback(material);

	if (material){
		material->PreEditChange(0);
		material->PostEditChange();
		
		if (postEditCallback)
			postEditCallback(material);
		//importer->registerMaterialPath(jsonMat.id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(material);
		matPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();

	return material;
}

UMaterial* MaterialBuilder::loadDefaultMaterial(){
	return nullptr;
}

UMaterialInstanceConstant* MaterialBuilder::importMaterialInstance(const JsonMaterial& jsonMat, JsonImporter *importer, JsonMaterialId matId){
	MaterialFingerprint fingerprint(jsonMat);

	FString matName = sanitizeObjectName(jsonMat.name);
	auto matPath = FPaths::GetPath(jsonMat.path);
	FString packagePath = buildPackagePath(
		matName, matPath, importer
	);

	FString defaultMatPath = TEXT("/JsonImport/exodusSolidMaterial");

	auto *baseMaterial = LoadObject<UMaterial>(nullptr, *defaultMatPath);
	if (!baseMaterial){
		UE_LOG(JsonLog, Warning, TEXT("Could not load default material \"%s\""));
	}

	//createPackage(
	auto matFactory = makeFactoryRootGuard<UMaterialInstanceConstantFactoryNew>();
	auto matInst = createAssetObject<UMaterialInstanceConstant>(matName, &matPath, importer, 
		[&](UMaterialInstanceConstant* inst){
			inst->PreEditChange(0);
			inst->PostEditChange();
			inst->MarkPackageDirty();
		}, 
		[&](UPackage* pkg) -> auto{
			matFactory->InitialParent = baseMaterial;
			auto result = (UMaterialInstanceConstant*)matFactory->FactoryCreateNew(
				UMaterialInstanceConstant::StaticClass(), pkg, *sanitizeObjectName(matName), RF_Standalone|RF_Public, 0, GWarn
			);

			setupMaterialInstance(result, jsonMat, importer, matId);

			return result;
		}, RF_Standalone|RF_Public
	);
	
	if (!matInst){
		UE_LOG(JsonLog, Warning, TEXT("Could not load mat instance \"%s\""), *jsonMat.name);
		return matInst;
	}

	return matInst;

	//FMaterialUtil
	//FMaterialUtilities


#if 0
	FString sanitizedMatName;
	FString sanitizedPackageName;

	UMaterial *existingMaterial = nullptr;
	UPackage *matPackage = importer->createPackage(
		jsonMat.name, jsonMat.path, importer->getAssetRootPath(), FString("Material"), 
		&sanitizedPackageName, &sanitizedMatName, &existingMaterial);

	if (existingMaterial){
		importer->registerMaterialPath(jsonMat.id, existingMaterial->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing material: %s (package %s)"), *sanitizedMatName, *sanitizedPackageName);
		return existingMaterial;
	}

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPackage, FName(*sanitizedMatName), RF_Standalone|RF_Public,
		0, GWarn);

	//stuff
	MaterialBuildData buildData(matId, importer);
	buildMaterial(material, jsonMat, fingerprint, buildData);

	if (material){
		material->PreEditChange(0);
		material->PostEditChange();

		importer->registerMaterialPath(jsonMat.id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(material);
		matPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();
#endif
	return nullptr;
}

void MaterialBuilder::setScalarParam(UMaterialInstanceConstant *matInst, const char *paramName, float val) const{
	check(matInst);
	check(paramName);

	FMaterialParameterInfo paramInfo(paramName);
	matInst->SetScalarParameterValueEditorOnly(paramInfo, val);
}

void MaterialBuilder::setVectorParam(UMaterialInstanceConstant *matInst, const char *paramName, FLinearColor val) const{
	check(matInst);
	check(paramName);

	FMaterialParameterInfo paramInfo(paramName);
	matInst->SetVectorParameterValueEditorOnly(paramInfo, val);
}

void MaterialBuilder::setVectorParam(UMaterialInstanceConstant *matInst, const char *paramName, FVector2D val) const{
	setVectorParam(matInst, paramName, FLinearColor(val.X, val.Y, 0.0f, 1.0f));
}

void MaterialBuilder::setVectorParam(UMaterialInstanceConstant *matInst, const char *paramName, FVector val) const{
	setVectorParam(matInst, paramName, FLinearColor(val.X, val.Y, val.Z, 1.0f));
}

void MaterialBuilder::setTexParam(UMaterialInstanceConstant *matInst, const char *paramName, UTexture *tex) const{
	check(matInst);
	check(paramName);

	FMaterialParameterInfo paramInfo(paramName);
	matInst->SetTextureParameterValueEditorOnly(paramInfo, tex);
}

void MaterialBuilder::setTexParam(UMaterialInstanceConstant *matInst, const char *paramName, int32 texId, const JsonImporter *importer) const{
	check(matInst);
	check(paramName);
	check(importer);

	auto tex = importer->getTexture(texId);
	if (!tex)
		return;

	setTexParam(matInst, paramName, tex);
}


bool MaterialBuilder::setStaticSwitch(FStaticParameterSet &paramSet, const char *switchName, bool newValue) const{
	check(switchName);
	auto name = FName(switchName);
	for(int i = 0; i < paramSet.StaticSwitchParameters.Num(); i++){
		auto &cur = paramSet.StaticSwitchParameters[i];
		if (cur.ParameterInfo.Name == switchName){
			cur.bOverride = true;
			cur.Value = newValue;
			return true;
		}
	}
	UE_LOG(JsonLog, Warning, TEXT("Could not find and set parameter \"%s\""), *name.ToString());
	return false;
}

void printMaterialInstanceData(UMaterialInstanceConstant *matInst, const FString &matInstName){
	check(matInst);
	UE_LOG(JsonLog, Log, TEXT("Printing parameters for matInst \"%s\""), *matInstName);

	FStaticParameterSet outParams;
	matInst->GetStaticParameterValues(outParams);

	UE_LOG(JsonLog, Log, TEXT("Num static params: %d"), outParams.StaticSwitchParameters.Num());
	for(int i = 0; i < outParams.StaticSwitchParameters.Num(); i++){
		const auto &cur = outParams.StaticSwitchParameters[i];
		auto guidStr = cur.ExpressionGUID.ToString();
		auto paramName = cur.ParameterInfo.Name.ToString();
		UE_LOG(JsonLog, Log, TEXT("param %d: override: %d, guid: %s, paramname: %s"),
			i, (int)cur.bOverride, *guidStr, *paramName);
	}

	//outParams.fin
	UE_LOG(JsonLog, Log, TEXT("Dumping scalars: %d"), matInst->ScalarParameterValues.Num());
	for(int i = 0; i < matInst->ScalarParameterValues.Num(); i++){
		const auto &cur = matInst->ScalarParameterValues[i];
		auto guidStr = cur.ExpressionGUID.ToString();
		auto paramName = cur.ParameterInfo.Name.ToString();
		auto strVal = cur.ParameterInfo.ToString();
		UE_LOG(JsonLog, Log, TEXT("Param %d: guid: %s; name: %s; str: %s"),
			i, *guidStr, *paramName, *strVal);
	}

	UE_LOG(JsonLog, Log, TEXT("Dumping vectors: %d"), matInst->VectorParameterValues.Num());
	for(int i = 0; i < matInst->VectorParameterValues.Num(); i++){
		const auto &cur = matInst->VectorParameterValues[i];
		auto guidStr = cur.ExpressionGUID.ToString();
		auto paramName = cur.ParameterInfo.Name.ToString();
		auto strVal = cur.ParameterInfo.ToString();
		UE_LOG(JsonLog, Log, TEXT("Param %d: guid: %s; name: %s; str: %s"),
			i, *guidStr, *paramName, *strVal);
	}

	UE_LOG(JsonLog, Log, TEXT("Dumping textures: %d"), matInst->TextureParameterValues.Num());
	for(int i = 0; i < matInst->TextureParameterValues.Num(); i++){
		const auto &cur = matInst->TextureParameterValues[i];
		auto guidStr = cur.ExpressionGUID.ToString();
		auto paramName = cur.ParameterInfo.Name.ToString();
		auto strVal = cur.ParameterInfo.ToString();
		UE_LOG(JsonLog, Log, TEXT("Param %d: guid: %s; name: %s; str: %s"),
			i, *guidStr, *paramName, *strVal);
	}
}

bool MaterialBuilder::setTexParams(UMaterialInstanceConstant *matInst,  FStaticParameterSet &paramSet, int32 texId, 
		const char *switchName, const char *texParamName, const JsonImporter *importer) const{
	check(matInst);
	check(importer);
	check(switchName);
	check(texParamName);

	auto tex = importer->getTexture(texId);
	if (!setStaticSwitch(paramSet, switchName, tex != nullptr))
		return false;
	setTexParam(matInst, texParamName, tex);
	return true;
}

void MaterialBuilder::setupMaterialInstance(UMaterialInstanceConstant *matInst, const JsonMaterial &jsonMat, JsonImporter *importer, JsonMaterialId matId){
	if (!matInst){
		UE_LOG(JsonLog, Warning, TEXT("Mat instance is null!"));
		return;
	}

	MaterialFingerprint fingerprint(jsonMat);

	//auto val = matInst->VectorParameterValues.AddDefaulted_GetRef();

	FStaticParameterSet outParams;
	matInst->GetStaticParameterValues(outParams);

	//that's a lot of parameters. (-_-)
	//Don't touch them without a GOOD reason.

	//albedo texture and color
	//albedoColor (c)
	setVectorParam(matInst, "albedoColor", jsonMat.colorGammaCorrected);
	//albedoTexEnabled (bool)
	//albedoTexture (tex2d)
	setTexParams(matInst, outParams, jsonMat.mainTexture, "albedoTexEnabled", "albedoTexture", importer);

	//main texutre offset
	//mainTextureTransformEnabled (bool)
	setStaticSwitch(outParams, "mainTextureTransformEnabled", fingerprint.mainTextureTransform);
	//mainTexOffset (vec2 as vec4)
	//mainTexScale(vec2 as vec4)
	setVectorParam(matInst, "mainTexOffset", jsonMat.mainTextureOffset);
	setVectorParam(matInst, "mainTexScale", jsonMat.mainTextureScale);

	//altSmoothnessSourceEnabled (bool)
	setStaticSwitch(outParams, "altSmoothnessSourceEnabled", fingerprint.altSmoothnessTexture);

	//detailAlbedo(tex2d)
	//detailAlbedoEnabled(bool)
	setTexParams(matInst, outParams, jsonMat.detailAlbedoTex, "detailAlbedoEnabled", "detailAlbedo", importer);
	//detailAlbedoOffset(vec2 - as vec4)
	//detailAlbedoScale (vec2 - as vec4)
	setVectorParam(matInst, "detailAlbedoScale", jsonMat.detailAlbedoScale);
	setVectorParam(matInst, "detailAlbedoOffset", jsonMat.detailAlbedoOffset);

	//detailTexTransformEnabled (bool)
	setStaticSwitch(outParams, "detailTexTransformEnabled", fingerprint.detailTextureTransform);

	//detialMask (tex2d)
	//detailMaskEnabled (bool)
	setTexParams(matInst, outParams, jsonMat.detailMaskTex, "detailMaskEnabled", "detialMask", importer);

	//detailNormalEnabled(bool)
	//detailNormalMap (tex2d)
	setTexParams(matInst, outParams, jsonMat.detailNormalMapTex, "detailNormalEnabled", "detailNormalMap", importer);

	//detailNormalScaleEnabled (fbool
	//detailNormalMapScale (float, bumpScale)
	setStaticSwitch(outParams, "detailNormalScaleEnabled", fingerprint.detailNormalMapScale);
	setScalarParam(matInst, "detailNormalMapScale", jsonMat.detailNormalMapScale);

	//detailUseUv0 (bool)
	setStaticSwitch(outParams, "detailUseUv0", fingerprint.secondaryUv == 0);
	//detailUseUv1 (bool)
	setStaticSwitch(outParams, "detailUseUv1", fingerprint.secondaryUv == 1);
	//detailUseUv2 (bool)
	setStaticSwitch(outParams, "detailUseUv2", fingerprint.secondaryUv == 2);
	//detailUseUv3 (bool)
	setStaticSwitch(outParams, "detailUseUv3", fingerprint.secondaryUv == 3);

	//emissionEnabled(bool)
	setStaticSwitch(outParams, "emissionEnabled", fingerprint.emissionEnabled);
	//emissiveColor(FlinearColor)
	setVectorParam(matInst, "emissiveColor", jsonMat.emissionColor);
	//emissionTexEnabled(bool)
	//emissiveTexture (tex2d)
	setTexParams(matInst, outParams, jsonMat.emissionTex, "emissionTexEnabled", "emissiveTexture", importer);

	//metallic (float)
	setScalarParam(matInst, "metallic", jsonMat.metallic);

	//metallicTex (tex2d)
	//metallicTexEnabled (bool)
	setTexParams(matInst, outParams, jsonMat.metallicTex, "metallicTexEnabled", "metallicTex", importer);

	//normalMapTexEnabled (bool)
	//normalMapTexture (tex2d)
	setTexParams(matInst, outParams, jsonMat.normalMapTex, "normalMapTexEnabled", "normalMapTexture", importer);

	//normalMapScale (float, bumpScale)
	//normalMapScaleEnabled (bool)
	setStaticSwitch(outParams, "normalMapScaleEnabled", fingerprint.normalMapIntensity);
	setScalarParam(matInst, "normalMapScale", jsonMat.bumpScale);

	//occlusionScaleEnabled(float)
	//occlusionScale (float)
	setStaticSwitch(outParams, "occlusionScaleEnabled", fingerprint.occlusionIntensity);
	setScalarParam(matInst, "occlusionScale", jsonMat.occlusionStrength);

	//occlusionTexEnabled (bool)
	//occlusionTex (tex2d)
	setTexParams(matInst, outParams, jsonMat.normalMapTex, "normalMapTexEnabled", "normalMapTexture", importer);

	//roughness (float)
	setScalarParam(matInst, "roughness", 1.0f - jsonMat.smoothness);//hmm...

	//specularColor (FlinearColor)
	setVectorParam(matInst, "specularColor", jsonMat.specularColorGammaCorrected);//hmm...

	//specularTexEnabled ( bool )
	//specularTex (tex2d)
	setTexParams(matInst, outParams, jsonMat.specularTex, "specularTexEnabled", "specularTex", importer);

	//specularWorkflowEnabled (bool, specularMode)
	setStaticSwitch(outParams, "specularWorkflowEnabled", fingerprint.specularModel);

	//transparencyEnabled (bool)
	setStaticSwitch(outParams, "transparencyEnabled", fingerprint.isAlphaBlendMode());

	//useOpacityMask (bool, switch on for cutout mode)
	setStaticSwitch(outParams, "useOpacityMask", fingerprint.isAlphaTestMode());

	matInst->UpdateStaticPermutation(outParams);

	//matInst->

	/*
	if (jsonMat.isTransparentQueue()){
		material->BlendMode = BLEND_Translucent;
		translucent = true;
	}
	if (jsonMat.isAlphaTestQueue())
		material->BlendMode = BLEND_Masked;
	if (jsonMat.isGeomQueue())
		material->BlendMode = BLEND_Opaque;

	bool needsOpacity = (jsonMat.isTransparentQueue() || jsonMat.isAlphaTestQueue()) && !jsonMat.isGeomQueue();
	if (!needsOpacity)
		return;

	if (translucent){
		material->TranslucencyLightingMode = TLM_SurfacePerPixelLighting;//TLM_Surface;
	}

	*/
	if (fingerprint.isAlphaBlendMode()){
		matInst->BasePropertyOverrides.bOverride_BlendMode;
		matInst->BasePropertyOverrides.BlendMode = BLEND_Translucent;
		//no translucency override, huh.
		//material->TranslucencyLightingMode = TLM_SurfacePerPixelLighting;//TLM_Surface;
	}
	else if (fingerprint.isAlphaTestMode()){
		matInst->BasePropertyOverrides.bOverride_BlendMode;
		matInst->BasePropertyOverrides.BlendMode = BLEND_Masked;
	}

	matInst->PostEditChange();
}
