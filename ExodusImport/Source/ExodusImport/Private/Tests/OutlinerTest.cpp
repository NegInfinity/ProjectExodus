#include "JsonImportPrivatePCH.h"
#include "OutlinerTest.h"
#include "UnrealUtilities.h"
#include "Factories/WorldFactory.h"
#include "GameFramework/Actor.h"
#include "Engine/PointLight.h"
#include "AssetRegistryModule.h"

using namespace UnrealUtilities;

void OutlinerTest::run(){
	auto factory = makeFactoryRootPtr<UWorldFactory>();
	FString worldName = TEXT("testWorld");
	FString baseDir = TEXT("/Game/Import/WorldTest");
	auto fullPath = FString::Printf(TEXT("%s/%s"), *baseDir, *worldName);

	auto worldPkg = createPackage(fullPath);
	auto world = CastChecked<UWorld>(
		factory->FactoryCreateNew(UWorld::StaticClass(), worldPkg, *worldName, RF_Standalone|RF_Public, 0, GWarn)
	);

	auto actor = world->SpawnActor<APointLight>();

	//huh. This actually calls for rename
	auto newName = TEXT("TestActor");
	actor->SetActorLabel(newName, false);
	/*
	Well. We COULD set the property directly, BUT the engine warns us not to do so.
	What now?
	*/

	/*
	auto renameResult = actor->Rename(newName, nullptr,  REN_DontCreateRedirectors);
	UE_LOG(JsonLog, Log, TEXT("Rename result: %d"), (int)renameResult);
	FPropertyChangedEvent PropertyEvent(FindField<UProperty>( AActor::StaticClass(), "ActorLabel"));
	actor->PostEditChangeProperty(PropertyEvent);
	*/

	//actor->setName

	if (worldPkg){
		FAssetRegistryModule::AssetCreated(world);
		//worldPkg->SetDirtyFlag(true);
		/*
		auto path = worldPkg->GetPathName();
		auto folderPath = FPaths::GetPath(path);
		auto contentPath = FPaths::ProjectContentDir();
		auto fullpath = FPackageName::LongPackageNameToFilename(outPackageName, FPackageName::GetAssetPackageExtension());

		UPackage::Save(worldPackage, newWorld, RF_Standalone|RF_Public, *fullpath);
		*/
	}

	/*
	auto world = createAssetObject(worldName, *fullPath, 0, 
		nullptr, 
		[&](auto pkg){
			return factory->FactoryCreateNew(UWorld::StaticClass(), pkg, *worldName, RF_Standalone|RF_Public, 0, GWarn);
		}
	);
	*/
}
