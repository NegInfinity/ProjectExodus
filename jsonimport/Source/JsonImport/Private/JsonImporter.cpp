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
#include "JsonObjects.h"

#define LOCTEXT_NAMESPACE "FJsonImportModule"

using namespace JsonObjects;

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
	const JsonValPtrs *resources = 0, *textures = 0, *materials = 0, *meshes = 0;

	loadArray(jsonData, resources, TEXT("resources"), TEXT("Resources"));
	//loadArray(jsonData, objects, TEXT("objects"), TEXT("Objects"));
	loadArray(jsonData, textures, TEXT("textures"), TEXT("Textures"));
	loadArray(jsonData, materials, TEXT("materials"), TEXT("Materials"));
	loadArray(jsonData, meshes, TEXT("meshes"), TEXT("Meshes"));

	assetCommonPath = findCommonPath(resources);

	loadTextures(textures);
	loadMaterials(materials);
	loadMeshes(meshes);
	//loadObjects(objects);
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
