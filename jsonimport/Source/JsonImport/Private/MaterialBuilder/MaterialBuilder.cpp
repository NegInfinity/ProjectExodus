#include "JsonImportPrivatePCH.h"
#include "MaterialBuilder.h"

#include "JsonImporter.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"

#include "MaterialTools.h"

using namespace MaterialTools;

void MaterialBuilder::buildMaterial(UMaterial* material, const JsonMaterial &jsonMat, 
		const MaterialFingerprint &fingerprint, JsonImporter *importer, MaterialBuildData &buildData){

	logValue("Transparent queue", jsonMat.isTransparentQueue());
	logValue("Alpha test queue", jsonMat.isAlphaTestQueue());
	logValue("Geom queue", jsonMat.isGeomQueue());

	//albedo
	UE_LOG(JsonLog, Log, TEXT("Creating albedo"));
	UMaterialExpressionTextureSample *albedoTexExpression = 0;
	UMaterialExpressionVectorParameter *albedoColorExpression = 0;

	UTexture *mainTexture = importer->getTexture(jsonMat.mainTexture);
	auto albedoSource = createMaterialInputMultiply(material, mainTexture, &jsonMat.colorGammaCorrected, material->BaseColor, 
		TEXT("Albedo(Texture)"), TEXT("Albedo(Color)"), &albedoTexExpression, &albedoColorExpression);

	if (jsonMat.useNormalMap){
		UE_LOG(JsonLog, Log, TEXT("Creating normal map"));

		auto normalMapTex = importer->getTexture(jsonMat.normalMapTex);
		createMaterialInput(material, normalMapTex, nullptr, material->Normal, true, TEXT("Normal"));
	}

	UE_LOG(JsonLog, Log, TEXT("Creating specular"));//TODO: connect specular alpha to smoothness

	UMaterialExpressionTextureSample *specTexExpression = 0;

	if (jsonMat.hasSpecular){
		auto specularTex = importer->getTexture(jsonMat.specularTex);
		createMaterialInput(material, specularTex, &jsonMat.specularColorGammaCorrected, material->Specular, false, 
			TEXT("Specular Color"), &specTexExpression);
	}
	else{
		// ?? Not sure if this is correct
		//material->Specular.Expression = albedoSource;
		//Nope, this is not correct. 
	}

	auto occlusionTex = importer->getTexture(jsonMat.occlusionTex);
	createMaterialInput(material, occlusionTex, nullptr, material->AmbientOcclusion, false, TEXT("Ambient Occlusion"));

	if (jsonMat.hasEmission){
		UE_LOG(JsonLog, Log, TEXT("Creating emissive"));

		UMaterialExpressionTextureSample *emissiveTexExp = 0;

		auto emissionTex = importer->getTexture(jsonMat.emissionTex);

		createMaterialInputMultiply(material, emissionTex, jsonMat.hasEmissionColor ? &jsonMat.emissionColor: 0, material->EmissiveColor, 
			TEXT("Emission Texture"), TEXT("Emission Color"));
		material->bUseEmissiveForDynamicAreaLighting = true;
		importer->registerEmissiveMaterial(buildData.matId);
	}

	//if (useMetallic){
	if (jsonMat.hasMetallic){
		UE_LOG(JsonLog, Log, TEXT("Creating metallic value"));
		createMaterialSingleInput(material, jsonMat.metallic, material->Metallic, TEXT("Metallic"));
	}

	UE_LOG(JsonLog, Log, TEXT("hasMetallic: %d; hasSpecular: %d"), (int)(jsonMat.hasSpecular), (int)jsonMat.hasMetallic);
	UE_LOG(JsonLog, Log, TEXT("specularMode:%d"), (int)(jsonMat.useSpecular));
	UE_LOG(JsonLog, Log, TEXT("specTex exiss:%d"), (int)(specTexExpression != nullptr));
	if (specTexExpression)
		UE_LOG(JsonLog, Log, TEXT("num outputs: %d"), specTexExpression->Outputs.Num());

	//if (useSpecular && (specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
	//useSpecular is false? the heck..
	//if ((specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
	if (jsonMat.hasSpecular && (specTexExpression != nullptr) && (specTexExpression->Outputs.Num() == 5)){
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
		createMaterialSingleInput(material, 1.0f - jsonMat.smoothness, material->Roughness, TEXT("Roughness"));
	}

	if (jsonMat.isTransparentQueue())
		material->BlendMode = BLEND_Translucent;
	if (jsonMat.isAlphaTestQueue())
		material->BlendMode = BLEND_Masked;
	if (jsonMat.isGeomQueue())
		material->BlendMode = BLEND_Opaque;

	bool needsOpacity = (jsonMat.isTransparentQueue() || jsonMat.isAlphaTestQueue()) && !jsonMat.isGeomQueue();
	if (needsOpacity){
		auto &opacityTarget = jsonMat.isTransparentQueue() ? material->Opacity: material->OpacityMask;

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
			UE_LOG(JsonLog, Warning, TEXT("Could not find matchin opacity source in material %s"), *jsonMat.name);
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
}

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
	MaterialBuildData buildData;
	buildData.matId = matId;
	buildMaterial(material, jsonMat, fingerprint, importer, buildData);

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
