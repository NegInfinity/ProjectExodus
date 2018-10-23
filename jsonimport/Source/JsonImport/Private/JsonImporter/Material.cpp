#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
	
#include "RawMesh.h"

#include "DesktopPlatformModule.h"

UMaterial* JsonImporter::loadMaterial(int32 id){
	UE_LOG(JsonLog, Log, TEXT("Looking for material %d"), id);
	if (id < 0){
		UE_LOG(JsonLog, Log, TEXT("Invalid id %d"), id);
		return 0;
	}

	if (!matIdMap.Contains(id)){
		UE_LOG(JsonLog, Log, TEXT("Id %d is not in the map"), id);
		return 0;
	}

	auto matPath = matIdMap[id];
	UMaterial *mat = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), 0, *matPath));
	UE_LOG(JsonLog, Log, TEXT("Material located"));
	return mat;
}

static FLinearColor applyGamma(const FLinearColor &arg){
	return FLinearColor(powf(arg.R, 2.2f), powf(arg.G, 2.2f), pow(arg.B, 2.2f), arg.A);
}

void JsonImporter::importMaterial(JsonObjPtr obj, int32 matId){
	UE_LOG(JsonLog, Log, TEXT("Importing material %d"), matId);

#define GETPARAM(name, op) auto name = op(obj, #name); logValue(#name, name);
	GETPARAM(name, getString)
	GETPARAM(id, getInt)
	GETPARAM(path, getString)
	GETPARAM(shader, getString)
	GETPARAM(renderQueue, getInt)

	GETPARAM(mainTexture, getInt)
	GETPARAM(mainTextureOffset, getVector2)
	GETPARAM(mainTextureScale, getVector2)
	GETPARAM(color, getLinearColor)

	GETPARAM(useNormalMap, getBool)
	GETPARAM(useAlphaTest, getBool)
	GETPARAM(useAlphaPremultiply, getBool)
	GETPARAM(useEmission, getBool)
	GETPARAM(hasEmission, getBool)
	GETPARAM(hasEmissionColor, getBool)
	GETPARAM(useParallax, getBool)
	GETPARAM(useDetailMap, getBool)
	GETPARAM(useMetallic, getBool)
	GETPARAM(hasMetallic, getBool)
	GETPARAM(useSpecular, getBool)
	GETPARAM(hasSpecular, getBool)
	GETPARAM(albedoTex, getInt)
	GETPARAM(specularTex, getInt)
	GETPARAM(normalMapTex, getInt)
	GETPARAM(occlusionTex, getInt)
	GETPARAM(parallaxTex, getInt)
	GETPARAM(emissionTex, getInt)
	GETPARAM(detailMaskTex, getInt)
	GETPARAM(detailAlbedoTex, getInt)
	GETPARAM(detailNormalMapTex, getInt)

	GETPARAM(alphaCutoff, getFloat)
	GETPARAM(smoothness, getFloat)
	GETPARAM(specularColor, getLinearColor)
	GETPARAM(metallic, getFloat)
	GETPARAM(bumpScale, getFloat)
	GETPARAM(parallaxScale, getFloat)
	GETPARAM(occlusionStrength, getFloat)
	GETPARAM(emissionColor, getLinearColor)
	GETPARAM(detailMapScale, getFloat)
	GETPARAM(secondaryUv, getFloat)
#undef GETPARAM
	FString sanitizedMatName;
	FString sanitizedPackageName;

	//Gamma correction.
	specularColor = applyGamma(specularColor);
	color = applyGamma(color);

	UMaterial *existingMaterial = nullptr;
	UPackage *matPackage = createPackage(
		name, path, assetRootPath, FString("Material"), 
		&sanitizedPackageName, &sanitizedMatName, &existingMaterial);
	if (existingMaterial){
		matIdMap.Add(id, existingMaterial->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing material: %s (package %s)"), *sanitizedMatName, *sanitizedPackageName);
		return;
	}

	bool isTransparentQueue = (renderQueue >= 3000) && (renderQueue < 4000);
	bool isAlphaTestQueue = (renderQueue >= 2450) && (renderQueue < 3000);
	bool isGeomQueue = (!isTransparentQueue && !isAlphaTestQueue) || 
		((renderQueue >= 2000) && (renderQueue < 2450));

	logValue("Transparent queue", isTransparentQueue);
	logValue("Alpha test queue", isAlphaTestQueue);
	logValue("Geom queue", isGeomQueue);

	auto matFactory = NewObject<UMaterialFactoryNew>();
	matFactory->AddToRoot();

	UMaterial* material = (UMaterial*)matFactory->FactoryCreateNew(
		UMaterial::StaticClass(), matPackage, FName(*sanitizedMatName), RF_Standalone|RF_Public,
		0, GWarn);

	//albedo
	UE_LOG(JsonLog, Log, TEXT("Creating albedo"));
	UMaterialExpressionTextureSample *albedoTexExpression = 0;
	UMaterialExpressionVectorParameter *albedoColorExpression = 0;
	auto albedoSource = createMaterialInputMultiply(material, mainTexture, &color, material->BaseColor, 
		TEXT("Albedo(Texture)"), TEXT("Albedo(Color)"), &albedoTexExpression, &albedoColorExpression);
	if (useNormalMap){
		UE_LOG(JsonLog, Log, TEXT("Creating normal map"));
		createMaterialInput(material, normalMapTex, nullptr, material->Normal, true, TEXT("Normal"));
	}

	UE_LOG(JsonLog, Log, TEXT("Creating specular"));//TODO: connect specular alpha to smoothness

	UMaterialExpressionTextureSample *specTexExpression = 0;//, *metalTexExpression = 0; no metallic textures, huh.

	if (hasSpecular){
		createMaterialInput(material, specularTex, &specularColor, material->Specular, false, 
			TEXT("Specular Color"), &specTexExpression);
	}
	else{
		// ?? Not sure if this is correct
		//material->Specular.Expression = albedoSource;
		//Nope, this is not correct. 
	}

	createMaterialInput(material, occlusionTex, nullptr, material->AmbientOcclusion, false, TEXT("Ambient Occlusion"));

	//useEmission = useEmission || (emissionTex >= 0);
	if (hasEmission){
		UE_LOG(JsonLog, Log, TEXT("Creating emissive"));

		UMaterialExpressionTextureSample *emissiveTexExp = 0;

		createMaterialInputMultiply(material, emissionTex, hasEmissionColor ? &emissionColor: 0, material->EmissiveColor, 
			TEXT("Emission Texture"), TEXT("Emission Color"));
		material->bUseEmissiveForDynamicAreaLighting = true;
		emissiveMaterials.Add(matId);
	}

	//if (useMetallic){
	if (hasMetallic){
		UE_LOG(JsonLog, Log, TEXT("Creating metallic value"));
		createMaterialSingleInput(material, metallic, material->Metallic, TEXT("Metallic"));
	}

	UE_LOG(JsonLog, Log, TEXT("hasMetallic: %d; hasSpecular: %d"), (int)(hasSpecular), (int)hasMetallic);
	UE_LOG(JsonLog, Log, TEXT("specularMode:%d"), (int)(useSpecular));
	UE_LOG(JsonLog, Log, TEXT("specTex exiss:%d"), (int)(specTexExpression != nullptr));
	if (specTexExpression)
		UE_LOG(JsonLog, Log, TEXT("num outputs: %d"), specTexExpression->Outputs.Num());

	//if (useSpecular && (specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
	//useSpecular is false? the heck..
	//if ((specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
	if (hasSpecular && (specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
		auto mulNode = NewObject<UMaterialExpressionMultiply>(material);
		material->Expressions.Add(mulNode);
		auto addNode = NewObject<UMaterialExpressionAdd>(material);
		material->Expressions.Add(addNode);
		addNode->ConstA = 1.0f;
		addNode->B.Expression = mulNode;
		mulNode->ConstA = -1.0f;

		specTexExpression->ConnectExpression(&mulNode->B, 4);
		material->Roughness.Expression = addNode;
	}
	else{
		createMaterialSingleInput(material, 1.0f - smoothness, material->Roughness, TEXT("Roughness"));
	}

	if (isTransparentQueue)
		material->BlendMode = BLEND_Translucent;
	if (isAlphaTestQueue)
		material->BlendMode = BLEND_Masked;
	if (isGeomQueue)
		material->BlendMode = BLEND_Opaque;

	bool needsOpacity = (isTransparentQueue || isAlphaTestQueue) && !isGeomQueue;
	if (needsOpacity){
		auto &opacityTarget = isTransparentQueue ? material->Opacity: material->OpacityMask;

		if (albedoTexExpression && albedoColorExpression){
			auto opacityMul = createExpression<UMaterialExpressionMultiply>(material);
			albedoTexExpression->ConnectExpression(&opacityMul->A, 4);
			albedoColorExpression->ConnectExpression(&opacityMul->B, 4);
			opacityTarget.Expression = opacityMul;
		}else if (albedoTexExpression != 0)
			albedoTexExpression->ConnectExpression(&opacityTarget, 4);
		else if (albedoColorExpression != 0)
			albedoColorExpression->ConnectExpression(&opacityTarget, 4);
		else{
			UE_LOG(JsonLog, Warning, TEXT("Could not find matchin opacity source in material %s"), *name);
		}
	}

	auto numExpressions = material->Expressions.Num();
	int expressionRows = (int)(sqrtf((float)numExpressions))+1;
	if (expressionRows != 0){
			for (int i = 0; i < numExpressions; i++){
					auto cur = material->Expressions[i];
					auto row = i / expressionRows;
					auto col = i % expressionRows;

					int32 size = 256;

					int32 x = (col - expressionRows) * size;
					int32 y = row * size;

					cur->MaterialExpressionEditorX = x;
					cur->MaterialExpressionEditorY = y;
			}
	}
	
	material->PreEditChange(0);
	material->PostEditChange();

	if (material){
		matIdMap.Add(id, material->GetPathName());
		FAssetRegistryModule::AssetCreated(material);
		matPackage->SetDirtyFlag(true);
	}

	matFactory->RemoveFromRoot();
}
