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

#include "Factories/WorldFactory.h"

#include "RawMesh.h"

#include "DesktopPlatformModule.h"
#include "FileHelpers.h"
#include "ObjectTools.h"
#include "PackageTools.h"

#include "UnrealUtilities.h"
#include "JsonObjects.h"

#define LOCTEXT_NAMESPACE "FJsonImportModule"

using namespace UnrealUtilities;
using namespace JsonObjects;

UWorld* JsonImporter::importScene(const JsonScene &scene, bool createWorld){
	const JsonValPtrs *sceneObjects = 0;

	bool editorMode = !createWorld;

	auto sceneName = scene.name;
	auto scenePath = scene.path;
	auto buildIndex = scene.buildIndex;
	
	if (scenePath.IsEmpty()){
		UE_LOG(JsonLog, Warning, TEXT("Empty scene path. Scene name  %s, scene path %s"), *sceneName, *scenePath);
	}
	else
		UE_LOG(JsonLog, Log, TEXT("Processing scene \"%s\"(%s)"), *sceneName, *scenePath);

	if (sceneName.IsEmpty()){
		sceneName = FString::Printf(TEXT("importedScene%s"), *genTimestamp());
	}
	if (scenePath.IsEmpty()){
		scenePath = FPaths::Combine(
			TEXT("!unnamedScenes"), sceneName
		);
	}

	if (!createWorld){
		ImportWorkData workData(GEditor->GetEditorWorldContext().World(), editorMode);
		loadObjects(scene.objects, workData);
		return nullptr;
	}

	auto result = importSceneObjectsAsWorld(scene.objects, sceneName, scenePath);

	return result;
}

UWorld* JsonImporter::importSceneObjectsAsWorld(const TArray<JsonGameObject> &sceneObjects, const FString &sceneName, const FString &scenePath){
	UWorldFactory *factory = NewObject<UWorldFactory>();
	factory->WorldType = EWorldType::Inactive;
	factory->bInformEngineOfWorld = true;
	factory->FeatureLevel = GEditor->DefaultWorldFeatureLevel;

	UWorld *existingWorld = 0;
	FString worldName = sceneName;
	FString worldFileName = scenePath;
	FString outWorldName, outPackageName;
	EObjectFlags flags = RF_Public | RF_Standalone;
	UPackage *worldPackage = 0;

	worldPackage = createPackage(worldName, worldFileName, assetRootPath, 
		FString("Level"), &outPackageName, &outWorldName, &existingWorld);

	if (existingWorld){
		UE_LOG(JsonLog, Warning, TEXT("World already exists for %s(%s)"), *sceneName, *scenePath);
		return nullptr;
	}

	UWorld *newWorld = CastChecked<UWorld>(factory->FactoryCreateNew(
		UWorld::StaticClass(), worldPackage, *outWorldName, flags, 0, GWarn));

	if (newWorld){
		ImportWorkData workData(newWorld, false);
		loadObjects(sceneObjects, workData);
	}

	if (worldPackage){
		//newWorld->PostEditChange();
		FAssetRegistryModule::AssetCreated(newWorld);
		worldPackage->SetDirtyFlag(true);
		auto path = worldPackage->GetPathName();
		auto folderPath = FPaths::GetPath(path);
		auto contentPath = FPaths::ProjectContentDir();
		auto fullpath = FPackageName::LongPackageNameToFilename(outPackageName, FPackageName::GetAssetPackageExtension());

		//UPackage::Save(worldPackage, newWorld, RF_Standalone|RF_Public, *fullpath);
	}
	return newWorld;
}

FString getWorldPackagePath(UWorld *world){
	if (!world)
		return FString();

	auto outer = world->GetOuter();
	if (!outer)
		return FString();

	auto packagePath = outer->GetPathName();
	auto filePath = FPackageName::LongPackageNameToFilename(packagePath);//no ext?
	UE_LOG(JsonLog, Log, TEXT("Path for the newly created world: \"%s\""), *filePath);

	return filePath;
}

void JsonImporter::importProject(const FString& filename){
	setupAssetPaths(filename);
	auto jsonData = loadJsonFromFile(filename);
	if (!jsonData){
		UE_LOG(JsonLog, Error, TEXT("Json loading failed, aborting. \"%s\""), *filename);
		return;
	}

	JsonProject project(jsonData);
	externResources = project.externResources;

	importResources(externResources);
	const auto& scenes = externResources.scenes;

	auto singleScene = externResources.scenes.Num() == 1;
	auto createWorldFlag = !singleScene;
	FString lastWorldPackage;
	FScopedSlowTask sceneProgress(scenes.Num(), LOCTEXT("Importing scenes", "Importing scenes"));

	StringArray importedWorlds;

	sceneProgress.MakeDialog();
	for(int i = 0; i < scenes.Num(); i++){
		const auto& sceneFile = scenes[i];
		auto curSceneData = loadExternResourceFromFile(sceneFile);//(*scenes)[i];
		if (!curSceneData.IsValid()){
			UE_LOG(JsonLog, Error, TEXT("Invalid scene data %d, file \"%s\""), i, *sceneFile);
		}
		else{
			JsonScene scene(curSceneData);
			bool createWorldRequired = false;
			if (singleScene){
				if (scene.containsTerrain()){
					//FMessageDialog::Debugf(TEXT("The scene you're importing contains terrain, and will be imported as a new level"));
					FMessageDialog::Debugf(LOCTEXT("Scene contains terrain", "The scene you're importing contains terrain, and will be imported as a new level"));
					createWorldRequired = true;
				}
			}
			auto curCreateFlag = createWorldFlag || createWorldRequired;
			auto curWorld = importScene(scene, curCreateFlag);//importScene(curSceneDataObj, true);
			auto curPath = getWorldPackagePath(curWorld);
			if (!curPath.IsEmpty())
				lastWorldPackage = curPath;
			if (curWorld && curCreateFlag){
				importedWorlds.Add(curWorld->GetPathName());
			}
		}
		sceneProgress.EnterProgressFrame();
	}

	if (importedWorlds.Num() > 0){
		FString text = TEXT("Scenes imported as:\n");
		for(const auto& cur: importedWorlds){
			text += FString::Printf(TEXT("%s\n"), *cur);
		}
		FMessageDialog::Debugf(FText::FromString(text));
	}
}
