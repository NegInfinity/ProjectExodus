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

	FString defaultMatPath = TEXT("JsonImport/exodusMaskMaterial.Material");

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

void MaterialBuilder::setupMaterialInstance(UMaterialInstanceConstant *matInst, const JsonMaterial &jsonMat, JsonImporter *importer, JsonMaterialId matId){
}
