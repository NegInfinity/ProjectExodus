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
#include "Engine/Classes/Components/ChildActorComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/WorldFactory.h"

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
#include "JsonObjects.h"

#include "Kismet2/KismetEditorUtilities.h"
#include "UnrealUtilities.h"
#include "JsonObjects/JsonTerrainData.h"

#define LOCTEXT_NAMESPACE "FJsonImportModule"

using namespace JsonObjects;
using namespace UnrealUtilities;
//using namespace MaterialTools;

FString JsonImporter::getProjectImportPath() const{
	auto result = getDefaultImportPath();
	if (result.Len() && sourceBaseName.Len())
		result = FPaths::Combine(*result, *sourceBaseName);
	return result;
}

void JsonImporter::importTerrainData(JsonObjPtr jsonData, JsonId terrainId, const FString &rootPath){
	//
	JsonTerrainData terrainData;
	terrainData.load(jsonData);

	terrainDataMap.Add(terrainId, terrainData);
	//binTerrainIdMap.Add(terrainData
}

void JsonImporter::loadTerrains(const JsonValPtrs* terrains){
	if (!terrains)
		return;

	FScopedSlowTask terProgress(terrains->Num(), LOCTEXT("Importing terrains", "Importing terrains"));
	terProgress.MakeDialog();
	JsonId id = 0;
	for(auto cur: *terrains){
		auto obj = cur->AsObject();
		auto curId= id;
		id++;
		if (!obj.IsValid())
			continue;

		importTerrainData(obj, curId, assetRootPath);
		terProgress.EnterProgressFrame(1.0f);
	}

}

void JsonImporter::loadTextures(const JsonValPtrs* textures){
	if (!textures)
		return;

	FScopedSlowTask texProgress(textures->Num(), LOCTEXT("Importing textures", "Importing textures"));
	texProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing textures"));
	for(auto cur: *textures){
		auto obj = cur->AsObject();
		if (!obj.IsValid())
			continue;
		importTexture(obj, assetRootPath);
		texProgress.EnterProgressFrame(1.0f);
	}
}

void JsonImporter::loadMaterials(const JsonValPtrs* materials){
	if (!materials)
		return;
	FScopedSlowTask matProgress(materials->Num(), LOCTEXT("Importing materials", "Importing materials"));
	matProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing materials"));
	int32 matId = 0;
	for(auto cur: *materials){
		auto obj = cur->AsObject();
		auto curId = matId;
		matId++;
		if (!obj.IsValid())
			continue;
		importMaterial(obj, curId);
		matProgress.EnterProgressFrame(1.0f);
	}
}

void JsonImporter::loadMeshes(const JsonValPtrs* meshes){
	if (!meshes)
		return;
	FScopedSlowTask meshProgress(meshes->Num(), LOCTEXT("Importing materials", "Importing meshes"));
	meshProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing meshes"));
	int32 meshId = 0;
	for(auto cur: *meshes){
		auto obj = cur->AsObject();
		auto curId = meshId;
		meshId++;
		if (!obj.IsValid())
			continue;
		importMesh(obj, curId);
		meshProgress.EnterProgressFrame(1.0f);
	}
}

void JsonImporter::loadObjects(const JsonValPtrs* objects, ImportWorkData &importData){
	if (!objects)
		return;
	FScopedSlowTask objProgress(objects->Num(), LOCTEXT("Importing objects", "Importing objects"));
	objProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Import objects"));
	int32 objId = 0;
	for(auto cur: *objects){
		auto obj = cur->AsObject();
		auto curId = objId;
		objId++;
		if (!obj.IsValid())
			continue;
		importObject(obj, objId, importData);
		objProgress.EnterProgressFrame(1.0f);
	}
}


void JsonImporter::importResources(JsonObjPtr jsonData){
	const JsonValPtrs *resources = 0, *textures = 0, 
		*materials = 0, *meshes = 0, *prefabs = 0, 
		*terrains = 0;

	loadArray(jsonData, resources, TEXT("resources"), TEXT("Resources"));
	loadArray(jsonData, textures, TEXT("textures"), TEXT("Textures"));
	loadArray(jsonData, materials, TEXT("materials"), TEXT("Materials"));
	loadArray(jsonData, meshes, TEXT("meshes"), TEXT("Meshes"));
	loadArray(jsonData, prefabs, TEXT("prefabs"), TEXT("Prefabs"));
	loadArray(jsonData, terrains, TEXT("terrains"), TEXT("Terrains"));

	assetCommonPath = findCommonPath(resources);

	loadTextures(textures);
	loadMaterials(materials);
	loadMeshes(meshes);
	importPrefabs(prefabs);
	loadTerrains(terrains);
}

JsonObjPtr JsonImporter::loadJsonFromFile(const FString &filename){
	FString jsonString;
	if (!FFileHelper::LoadFileToString(jsonString, *filename)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load json file \"%s\""), *filename);
		return 0;
	}

	UE_LOG(JsonLog, Log, TEXT("Loaded json file \"%s\""), *filename);
	JsonReaderRef reader = TJsonReaderFactory<>::Create(jsonString);

	JsonObjPtr jsonData = MakeShareable(new FJsonObject());
	if (!FJsonSerializer::Deserialize(reader, jsonData)){
		UE_LOG(JsonLog, Warning, TEXT("Could not parse json file \"%s\""), *filename);
		return 0;
	}
	return jsonData;
}

void JsonImporter::setupAssetPaths(const FString &jsonFilename){
	assetRootPath = FPaths::GetPath(jsonFilename);
	sourceBaseName = FPaths::GetBaseFilename(jsonFilename);
}

void JsonImporter::registerMaterialPath(int32 id, FString path){
	if (matIdMap.Contains(id)){
		UE_LOG(JsonLog, Warning, TEXT("DUplicate material registration for id %d, path \"%s\""), id, *path);
	}
	matIdMap.Add(id, path);
}

void JsonImporter::registerEmissiveMaterial(int32 id){
	emissiveMaterials.Add(id);
}

FString JsonImporter::getMeshPath(JsonId id) const{
	auto result = meshIdMap.Find(id);
	if (result)
		return *result;
	return FString();
}

UStaticMesh* JsonImporter::loadStaticMeshById(JsonId id) const{
	auto path = meshIdMap.Find(id);
	if (!path)
		return nullptr;
	auto result = LoadObject<UStaticMesh>(0, **path);
	return result;
}
