#include "JsonImportPrivatePCH.h"
#include "PrefabBuilder.h"
#include "Factories/WorldFactory.h"
#include "Editor.h"
#include "JsonImporter.h"
#include "StrongObjectPtr.h"
#include "UnrealEd/Public/Kismet2/KismetEditorUtilities.h"
#include "AssetRegistryModule.h"

void PrefabBuilder::sanityCheck(const JsonPrefabData &prefabData){
	int numRoots = 0;
	for(const auto& cur: prefabData.objects){
		if (!cur.hasParent())
			numRoots++;
	}

	check(numRoots <= 1);
}

void PrefabBuilder::importPrefab(const JsonPrefabData& prefab, JsonImporter *importer){
	check(importer);

	if (prefab.objects.Num() <= 0){
		UE_LOG(JsonLogPrefab, Warning, TEXT("No objects in prefab %s(%s)"), *prefab.name, *prefab.path);
		return;
	}

	auto factory = TStrongObjectPtr<UWorldFactory>(NewObject<UWorldFactory>());
	factory->WorldType = EWorldType::Inactive;
	factory->bInformEngineOfWorld = true;
	factory->FeatureLevel = GEditor->DefaultWorldFeatureLevel;
	EObjectFlags flags = RF_Public | RF_Standalone;

	auto worldPkg = TStrongObjectPtr<UPackage>(CreatePackage(0, 0));

	auto tmpWorld = TStrongObjectPtr<UWorld>(
		CastChecked<UWorld>(
			factory->FactoryCreateNew(UWorld::StaticClass(), worldPkg.Get(), TEXT("Temporary"), flags, 0, GWarn)
		)
	);

	auto rootPath = importer->getAssetRootPath();
	FString blueprintName;
	FString packageName;
	UPackage *existingPackage = 0;
	UPackage *blueprintPackage = importer->createPackage(
		prefab.name, prefab.path, rootPath, 
		FString("Bluerint"), &packageName, &blueprintName, &existingPackage
	);

	if (existingPackage){
		UE_LOG(JsonLogPrefab, Warning, TEXT("Package already eixsts for %s (%s), cannot continue"), *prefab.name, *prefab.path);
		return;
	}

	ImportContext workData(tmpWorld.Get(), false, &prefab.objects);
	for(const auto& cur: prefab.objects){
		importer->importObject(cur, workData, true);
	}

	sanityCheck(prefab);

	AActor *rootActor = nullptr;
	/*
	auto rootJson = workData.findJsonObject(0);
	check(rootJson);
	auto rootObject = workData.createBlankActor(*rootJson);
	check(rootObject.actor != nullptr);
	rootActor = rootObject.actor;
	*/

	auto foundRootObject = workData.findImportedObject(0);
	check(foundRootObject != nullptr);
	auto rootObject = *foundRootObject;
	check(rootObject.isValid());

	if (rootObject.hasActor()){
		rootActor = rootObject.actor;
	}
	else{
		check(prefab.hasObjects());
		auto newRoot = workData.createBlankActor(prefab.objects[0]);
		rootObject.attachTo(&newRoot);
		check(newRoot.actor);
		rootActor = newRoot.actor;
		rootObject = newRoot;
	}

	auto *createdBlueprint = FKismetEditorUtilities::CreateBlueprintFromActor(FName(*blueprintName), blueprintPackage, rootActor, true, true);
	UE_LOG(JsonLogPrefab, Warning, TEXT("Created blueprint: %x"), createdBlueprint);
	if (createdBlueprint){
		FAssetRegistryModule::AssetCreated(createdBlueprint);
		blueprintPackage->SetDirtyFlag(true);
	}
}

#if 0
void JsonImporter::importPrefab(const JsonPrefabData& prefab){
#ifdef JSON_DISABLE_PREFAB_IMPORT
	UE_LOG(JsonLogPrefab, Warning, TEXT("Prefab import is currently disabled"));
	return;
#endif

	/*
	This is definitely broken and therefore disabled.
	*/
#if 0 
	if (prefab.objects.Num() <= 0){
		UE_LOG(JsonLogPrefab, Warning, TEXT("No objects in prefab %s(%s)"), *prefab.name, *prefab.path);
		return;
	}

	UWorldFactory *factory = NewObject<UWorldFactory>();
	factory->WorldType = EWorldType::Inactive;
	factory->bInformEngineOfWorld = true;
	factory->FeatureLevel = GEditor->DefaultWorldFeatureLevel;
	EObjectFlags flags = RF_Public | RF_Standalone;
	//EObjectFlags flags = RF_Public | RF_Transient;

	UPackage *worldPkg = CreatePackage(0, 0);

	UWorld *tmpWorld = CastChecked<UWorld>(
		factory->FactoryCreateNew(UWorld::StaticClass(), worldPkg, TEXT("Temporary"), flags, 0, GWarn)
		);

	const FString &rootPath = assetRootPath;
	FString blueprintName;
	FString packageName;
	UPackage *existingPackage = 0;
	UPackage *blueprintPackage = createPackage(prefab.name, prefab.path, rootPath, 
		FString("Bluerint"), &packageName, &blueprintName, &existingPackage);

	if (existingPackage){
		UE_LOG(JsonLogPrefab, Warning, TEXT("Package already eixsts for %s (%s), cannot continue"), *prefab.name, *prefab.path);
		return;
	}

	int objId = 0;
	ImportWorkData workData(tmpWorld, false, true);
	for(const auto& cur: prefab.objects){
		importObject(cur, objId, workData);
		objId++;
	}

	const auto &firstObject = prefab.objects[0];
	ImportedObject firstImportedGameObject(nullptr, nullptr);

	ImportedObject *rootObject = 0;
	//AActor *rootActor = 0;
	//if (workData.objectActors.Contains(0)){
	if (workData.importedObjects.Contains(0)){
		//rootActor = workData.objectActors[0];
		rootObject = &workData.importedObjects[0];
	}
	else{
		FTransform firstObjectTransform;
		firstObjectTransform.SetFromMatrix(firstObject.ueWorldMatrix);
		//errrm? Why isn't it being registered afterwards?
		auto *rootActor = createActor<AActor>(workData, firstObjectTransform, TEXT("AActor"));
		firstImportedGameObject = ImportedObject(rootActor);
		rootObject = &firstImportedGameObject;
	}

	//UE_LOG(JsonLogPrefab, Log, TEXT("Attaching actors. %d actors present"), workData.rootActors.Num());
	UE_LOG(JsonLogPrefab, Log, TEXT("Attaching actors. %d actors present"), workData.rootObjects.Num());

	//TArray<AActor*> prefabActors = workData.rootActors;
	auto prefabObjects = workData.rootObjects;
	TMap<AActor*, USceneComponent*> childToParent;
	for(int actorIndex = 0; actorIndex < prefabActors.Num(); actorIndex++){
		UE_LOG(JsonLogPrefab, Log, TEXT("Procesrsing actor %d out of %d"), actorIndex, prefabActors.Num());
		auto curActor = prefabActors[actorIndex];
		if (!curActor)
			continue;

		UE_LOG(JsonLogPrefab, Log, TEXT("Actor name: %s"), *curActor->GetActorLabel());

		/*
		TArray<UChildActorComponent*> childActorComponents;
		curActor->GetComponents<UChildActorComponent>(childActorComponents, false);
		UE_LOG(JsonLogPrefab, Log, TEXT("Found %d child actor components"), childActorComponents.Num())
		*/
		TArray<AActor*> childActors;
		curActor->GetAttachedActors(childActors);
		UE_LOG(JsonLogPrefab, Log, TEXT("Found %d child child actors"), childActors.Num())

			auto rootComp = rootActor->GetRootComponent();
		for(auto childActor: childActors){
			if (!childActor)
				continue;
			UE_LOG(JsonLogPrefab, Log, TEXT("Adding child actor: %s"), *childActor->GetActorLabel())
				prefabActors.Push(childActor);
		}

		if (curActor == rootActor){
			UE_LOG(JsonLogPrefab, Log, TEXT("Root actor/component detected. Child actors skipped"))
				continue;
		}

		TArray<USceneComponent*> components;
		curActor->GetComponents<USceneComponent>(components, false);
		UE_LOG(JsonLogPrefab, Log, TEXT("Processing actor components for actor %s. %d components found"), *curActor->GetActorLabel(), prefabActors.Num())

			for(int compIndex = 0; compIndex < components.Num(); compIndex++){
				UE_LOG(JsonLogPrefab, Log, TEXT("Processing component %d out of %d"), compIndex, components.Num());
				auto curComp = components[compIndex];
				if (!curComp){
					continue;
				}
				UE_LOG(JsonLogPrefab, Log, TEXT("Component name: %s"), *curComp->GetName());

				bool result = curComp->AttachToComponent(rootComp, FAttachmentTransformRules::KeepWorldTransform);
				UE_LOG(JsonLogPrefab, Log, TEXT("Attach component result: %d"), (int)result);
				UE_LOG(JsonLogPrefab, Log, TEXT("Root %x (%s), component: %x (%s)"), 
					rootComp, rootComp ? *rootComp->GetName(): TEXT("Null"), curComp, curComp? *curComp->GetName(): TEXT("Null"));
				if (!result)
					UE_LOG(JsonLogPrefab, Warning, TEXT("Attach failed"));
			}
	}

	auto *createdBlueprint = FKismetEditorUtilities::CreateBlueprintFromActor(FName(*blueprintName), blueprintPackage, rootActor, true, true);
	UE_LOG(JsonLogPrefab, Warning, TEXT("Created blueprint: %x"), createdBlueprint);
	if (createdBlueprint){
		FAssetRegistryModule::AssetCreated(createdBlueprint);
		blueprintPackage->SetDirtyFlag(true);
	}
#endif
}
#endif