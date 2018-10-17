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

#define LOCTEXT_NAMESPACE "FJsonImportModule"

void JsonImporter::importScene(JsonObjPtr sceneDataObj, bool createWorld){
	const JsonValPtrs *sceneObjects = 0;

	auto sceneName = getString(sceneDataObj, "name");
	auto scenePath = getString(sceneDataObj, "path");
	auto buildIndex = getInt(sceneDataObj, "buildIndex");

	if (scenePath.IsEmpty()){
		UE_LOG(JsonLog, Log, TEXT("Empty scene path. Scene name  %s, scene path %s"), *sceneName, *scenePath);
	}

	loadArray(sceneDataObj, sceneObjects, TEXT("objects"), TEXT("Scene objects"));
	if (!createWorld){
		auto world = GEditor->GetEditorWorldContext().World();
		loadObjects(sceneObjects, world);
	}
	else{
		UWorld *existingWorld = 0;
		FString worldName = sceneName;
		FString worldFileName = scenePath;
		FString packageName;
		FString outWorldName, outPackageName;
		UPackage *worldPackage = createPackage(worldName, worldFileName, assetRootPath, 
			FString("Level"), &outPackageName, &outWorldName, &existingWorld);

		UE_LOG(JsonLog, Log, TEXT("Creating world package. Name %s, package %s"), *outWorldName, *outPackageName);

		auto world = UWorld::CreateWorld(EWorldType::None, true, FName(*outWorldName), worldPackage);
		//loadObjects(sceneObjects, world);

		//GEditor->

		UE_LOG(JsonLog, Log, TEXT("World created: %x"), world);
		UE_LOG(JsonLog, Log, TEXT("World current level: %x"), world->GetCurrentLevel());
		UE_LOG(JsonLog, Log, TEXT("World persistent level: %x"), world->PersistentLevel);

		if (world){
			FAssetRegistryModule::AssetCreated(world);
			worldPackage->SetDirtyFlag(true);
		}
	}
}

void JsonImporter::importProject(const FString& filename){
	setupAssetPaths(filename);
	auto jsonData = loadJsonFromFile(filename);
	if (!jsonData){
		UE_LOG(JsonLog, Error, TEXT("Json loading failed, aborting. \"%s\""), *filename);
		return;
	}

	auto configPtr = getObject(jsonData, "config");
	const JsonValPtrs *scenes;
	//auto scenesPtr = jsonData->GetArrayField("scenes");
	loadArray(jsonData, scenes, "scenes");
	auto resources = getObject(jsonData, "resources");//jsonData->GetObjectField("resources");

	if (configPtr && scenes && resources){
		UE_LOG(JsonLog, Display, TEXT("Project nodes detected in file \"%s\". Loading as project"), *filename);
		importResources(resources);

		UE_LOG(JsonLog, Display, TEXT("%d scenes found in file \"%s\"."), scenes->Num(), *filename);
		if (scenes->Num() == 1){
			JsonValPtr curSceneData = (*scenes)[0];
			auto curSceneDataObj = curSceneData->AsObject();
			if (!curSceneDataObj){
				UE_LOG(JsonLog, Error, TEXT("Invalid scene data"));
				return;
			}			
			importScene(curSceneDataObj, false);
			return;
		}

		int numScenes = 0;
		FScopedSlowTask sceneProgress(scenes->Num(), LOCTEXT("Importing scenes", "Importing scenes"));
		sceneProgress.MakeDialog();
		for(int i = 0; i < scenes->Num(); i++){
			JsonValPtr curSceneData = (*scenes)[i];
			auto curSceneDataObj = curSceneData->AsObject();
			if (curSceneDataObj){
				//auto world = GEditor->GetEditorWorldContext().World();
				importScene(curSceneDataObj, true);
				//UE_LOG(JsonLog, Warning, TEXT("Only one scene is currently supported"));
				numScenes++;
				/*if (numScenes > 2)
					break;*/
			}
			sceneProgress.EnterProgressFrame();
		}
		//project
	}
	else{
		UE_LOG(JsonLog, Warning, TEXT("Project nodes not found. Attempting to load as scene"), *filename);
		importResources(jsonData);

		const JsonValPtrs *objects = 0;
		loadArray(jsonData, objects, TEXT("objects"), TEXT("Objects"));

		auto world = GEditor->GetEditorWorldContext().World();
		loadObjects(objects, world);
	}
}

void JsonImporter::importScene(const FString& filename){
	setupAssetPaths(filename);

	auto jsonData = loadJsonFromFile(filename);
	if (!jsonData){
		UE_LOG(JsonLog, Error, TEXT("Json loading failed, aborting. \"%s\""), *filename);
		return;
	}

	importResources(jsonData);

	const JsonValPtrs *objects = 0;
	loadArray(jsonData, objects, TEXT("objects"), TEXT("Objects"));
	auto world = GEditor->GetEditorWorldContext().World();
	loadObjects(objects, world);
}
