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

UMaterial* MaterialBuilder::importMasterMaterial(const JsonMaterial& jsonMat, JsonImporter *importer){
	MaterialFingerprint fingerprint(jsonMat);

	FString sanitizedMatName;
	FString sanitizedPackageName;

	UMaterial *existingMaterial = nullptr;
	UPackage *matPackage = importer->createPackage(
		jsonMat.name, jsonMat.path, importer->getAssetRootPath(), FString("Material"), 
		&sanitizedPackageName, &sanitizedMatName, &existingMaterial);

	if (existingMaterial){
		//importer->registerMasterMaterialPath(jsonMat.id, existingMaterial->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing material: %s (package %s)"), *sanitizedMatName, *sanitizedPackageName);
		return existingMaterial;
	}

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPackage, FName(*sanitizedMatName), RF_Standalone|RF_Public,
		0, GWarn);

	//stuff
	//MaterialBuildData buildData(matId, importer);
	MaterialBuildData buildData(jsonMat.id, importer);
	buildMaterial(material, jsonMat, fingerprint, buildData);

	if (material){
		material->PreEditChange(0);
		material->PostEditChange();

		//importer->registerMasterMaterialPath(jsonMat.id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(material);
		matPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();

	return material;
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

UMaterial* MaterialBuilder::getBaseMaterial(const JsonMaterial &jsonMat) const{
	auto baseMaterialPath = getBaseMaterialPath(jsonMat);
	UE_LOG(JsonLog, Log, TEXT("Loading base material %s"), *baseMaterialPath);
	auto *baseMaterial = LoadObject<UMaterial>(nullptr, *baseMaterialPath);
	if (!baseMaterial){
		UE_LOG(JsonLog, Error, TEXT("Could not load default material \"%s\""));
	}
	return baseMaterial;
}	

FString MaterialBuilder::getBaseMaterialPath(const JsonMaterial &jsonMat) const{
	FString defaultMatPath = TEXT("/JsonImport/exodusSolidMaterial");
	FString transparentMatPath = TEXT("/JsonImport/exodusBlendMaterial");
	FString maskedMatPath = TEXT("/JsonImport/exodusMaskMaterial");
	//UE_LOG(JsonLog, Log, TEXT("Selecting default mat path for: %s(%s)"), *jsonMat.name, *jsonMat.path);

	auto baseMaterialPath = defaultMatPath;

	/*
	So I've run into a material that is either transparent or cutout but is placed at geom queue. 
	Hence the new variables.
	*/

	if (jsonMat.heuristicIsTransparent()){
		baseMaterialPath = transparentMatPath;
	}
	if (jsonMat.heuristicIsCutout()){
		baseMaterialPath = maskedMatPath;
	}
	UE_LOG(JsonLog, Log, TEXT("Base material \"%s\" selected for material %d(%s)"), *baseMaterialPath, jsonMat.id, *jsonMat.name);
	return baseMaterialPath;
}

UMaterialInstanceConstant* MaterialBuilder::createMaterialInstance(const FString& name, const FString *dirPath, UMaterial* baseMaterial, JsonImporter *importer, 
		std::function<void(UMaterialInstanceConstant* matInst)> postConfig){
	check(baseMaterial);

	FString matName = sanitizeObjectName(name);
	FString pkgName = sanitizeObjectName(name + TEXT("_MatInstnace"));

	auto pkgPath = dirPath? *dirPath: FString();
	auto matPath = FPaths::GetPath(pkgPath);

	auto matFactory = makeFactoryRootGuard<UMaterialInstanceConstantFactoryNew>();
	auto matInst = createAssetObject<UMaterialInstanceConstant>(pkgName, &matPath, importer, 
		[&](UMaterialInstanceConstant* inst){
			inst->PreEditChange(0);
			inst->PostEditChange();
			inst->MarkPackageDirty();
		}, 
		[&](UPackage* pkg, auto sanitizedName) -> auto{
			matFactory->InitialParent = baseMaterial;
			auto result = (UMaterialInstanceConstant*)matFactory->FactoryCreateNew(
				UMaterialInstanceConstant::StaticClass(), pkg, 
				*sanitizedName,
				//*sanitizeObjectName(matName), 
				RF_Standalone|RF_Public, 0, GWarn
			);

			if (postConfig)
				postConfig(result);

			return result;
		}, RF_Standalone|RF_Public
	);
	
	if (!matInst){
		UE_LOG(JsonLog, Warning, TEXT("Could not load create material instance \"%s\""), *name);
		return matInst;
	}

	return matInst;
}

UMaterialInstanceConstant* MaterialBuilder::importMaterialInstance(const JsonMaterial& jsonMat, JsonImporter *importer){
	MaterialFingerprint fingerprint(jsonMat);

	auto unrealName = jsonMat.getUnrealMaterialName();

	auto baseMaterialPath = getBaseMaterialPath(jsonMat);
	UE_LOG(JsonLog, Log, TEXT("Loading base material %s"), *baseMaterialPath);
	auto *baseMaterial = LoadObject<UMaterial>(nullptr, *baseMaterialPath);
	if (!baseMaterial){
		UE_LOG(JsonLog, Warning, TEXT("Could not load default material \"%s\""));
	}

	//return createMaterialInstance(jsonMat.name, &jsonMat.path, baseMaterial, importer, 
	return createMaterialInstance(unrealName, &jsonMat.path, baseMaterial, importer, 
		[&](auto newInst){
			setupMaterialInstance(newInst, jsonMat, importer);
		}
	);
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

void MaterialBuilder::setupMaterialInstance(UMaterialInstanceConstant *matInst, const JsonMaterial &jsonMat, JsonImporter *importer){
	if (!matInst){
		UE_LOG(JsonLog, Warning, TEXT("Mat instance is null!"));
		return;
	}

	MaterialFingerprint fingerprint(jsonMat);

	//auto val = matInst->VectorParameterValues.AddDefaulted_GetRef();

	FStaticParameterSet outParams;
	matInst->GetStaticParameterValues(outParams);

/*
=======================
	that's a lot of parameters. (-_-)
	Don't touch them without a GOOD reason.
=======================
*/

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
	setTexParams(matInst, outParams, jsonMat.occlusionTex, "occlusionTexEnabled", "occlusionTex", importer);

	//roughness (float)
	setScalarParam(matInst, "roughness", 1.0f - jsonMat.smoothness);//hmm...

	//glossMapScale (float0
	setScalarParam(matInst, "glossMapScale", jsonMat.smoothnessScale);

	//specularColor (FlinearColor)
	setVectorParam(matInst, "specularColor", jsonMat.specularColorGammaCorrected);//hmm...

	//specularTexEnabled ( bool )
	//specularTex (tex2d)
	setTexParams(matInst, outParams, jsonMat.specularTex, "specularTexEnabled", "specularTex", importer);

	//specularWorkflowEnabled (bool, specularMode)
	setStaticSwitch(outParams, "specularWorkflowEnabled", fingerprint.specularModel);

	//transparencyEnabled (bool)
	//setStaticSwitch(outParams, "transparencyEnabled", jsonMat.isTransparentQueue() || jsonMat.isAlphaTestQueue());//fingerprint.isAlphaBlendMode());
	//setStaticSwitch(outParams, "transparencyEnabled", jsonMat.needsTransparencyFlag());//fingerprint.isAlphaBlendMode());
	setStaticSwitch(outParams, "transparencyEnabled", jsonMat.heuristicNeedsTransparentFlag());//fingerprint.isAlphaBlendMode());

	//useOpacityMask (bool, switch on for cutout mode)
	setStaticSwitch(outParams, "useOpacityMask", jsonMat.isAlphaTestQueue());//fingerprint.isAlphaTestMode());

	matInst->UpdateStaticPermutation(outParams);
	//matInst->InitStaticPermutation();
	matInst->PostEditChange();

	/*
	if (jsonMat.isTransparentQueue()){
		matInst->BasePropertyOverrides.bOverride_BlendMode = true;
		matInst->BasePropertyOverrides.BlendMode = BLEND_Translucent;
		//translucent = true;
	}
	if (jsonMat.isAlphaTestQueue()){
		matInst->BasePropertyOverrides.bOverride_BlendMode = true;
		matInst->BasePropertyOverrides.BlendMode = BLEND_Masked;
	}
	if (jsonMat.isGeomQueue()){
		matInst->BasePropertyOverrides.bOverride_BlendMode = true;
		matInst->BasePropertyOverrides.BlendMode = BLEND_Opaque;
	}
	*/
}

/*
UMaterialInstanceConstant* MaterialBuilder::createMaterialInstnace(const FString& name, const FString *dirPath, UMaterial* baseMaterial, JsonImporter *importer, 
		std::function<void(UMaterialInstanceConstant* matInst)> postConfig){
	return nullptr;
}
*/