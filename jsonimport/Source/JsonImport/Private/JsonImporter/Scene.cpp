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
		for(int i = 0; i < scenes->Num(); i++){
			JsonValPtr curSceneData = (*scenes)[i];
			auto curSceneDataObj = curSceneData->AsObject();
			if (!curSceneDataObj)
				continue;

			const JsonValPtrs *sceneObjects = 0;
			loadArray(curSceneDataObj, sceneObjects, TEXT("objects"), TEXT("Scene objects"));
			loadObjects(sceneObjects);

			UE_LOG(JsonLog, Warning, TEXT("Only one scene is currently supported"));
			break;
		}
		//project
	}
	else{
		UE_LOG(JsonLog, Warning, TEXT("Project nodes not found. Attempting to load as scene"), *filename);
		importResources(jsonData);

		const JsonValPtrs *objects = 0;
		loadArray(jsonData, objects, TEXT("objects"), TEXT("Objects"));
		loadObjects(objects);
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
	loadObjects(objects);
}
