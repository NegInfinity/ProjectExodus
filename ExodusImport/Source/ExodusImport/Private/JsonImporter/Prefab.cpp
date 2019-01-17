#include "JsonImportPrivatePCH.h"
#include "JsonImporter.h"

#define JSON_DISABLE_PREFAB_IMPORT

/*
Copied and modified from Kismet
*/
UBlueprint* createBlueprintFromActor(const FName blueprintName, UObject* outer, AActor* actor, 
		const bool bReplaceActor, bool bKeepMobility, bool openEditor){
	UBlueprint* result = nullptr;
	if (!outer)
		return nullptr;
	if (!actor)
		return nullptr;

	result = FKismetEditorUtilities::CreateBlueprint(actor->GetClass(), 
		outer, blueprintName, EBlueprintType::BPTYPE_Normal, 
		UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), 
		FName("CreateFromActor")
	);

	if (!result)
		return nullptr;

	FAssetRegistryModule::AssetCreated(result);

	outer->MarkPackageDirty();

	// If the source Actor has Instance Components we need to translate these in to SCS Nodes
	if (actor->GetInstanceComponents().Num() > 0){
		FKismetEditorUtilities::AddComponentsToBlueprint(result, actor->GetInstanceComponents(), false,  (USCS_Node*)nullptr, bKeepMobility);
	}

	if (result->GeneratedClass != nullptr){
		AActor* CDO = CastChecked<AActor>(result->GeneratedClass->GetDefaultObject());
		const auto CopyOptions = (EditorUtilities::ECopyOptions::Type)
			(EditorUtilities::ECopyOptions::OnlyCopyEditOrInterpProperties | EditorUtilities::ECopyOptions::PropagateChangesToArchetypeInstances);
		EditorUtilities::CopyActorProperties(actor, CDO, CopyOptions);

		if (USceneComponent* dstSceneRoot = CDO->GetRootComponent()){
			auto component = dstSceneRoot;
			component->RelativeLocation = FVector::ZeroVector;
			component->RelativeRotation = FRotator::ZeroRotator;
			component->SetupAttachment(nullptr);

			//FDirectAttachChildrenAccessor::Get(Component).Empty();
			//component->AttachChildren.Empty();

			component->InvalidateLightingCache();

			//FResetSceneComponentAfterCopy::Reset(DstSceneRoot);

			// Copy relative scale from source to target.
			if (USceneComponent* srcSceneRoot = actor->GetRootComponent()){
				dstSceneRoot->RelativeScale3D = srcSceneRoot->RelativeScale3D;
			}
		}
	}

	FKismetEditorUtilities::CompileBlueprint(result);

	if (result && openEditor){
		// Open the editor for the new blueprint
		FAssetEditorManager::Get().OpenEditorForAsset(result);
	}
	return result;
}


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
	ImportedGameObject firstImportedGameObject(nullptr, nullptr);

	ImportedGameObject *rootObject = 0;
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
		firstImportedGameObject = ImportedGameObject(rootActor);
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

void JsonImporter::importPrefabs(const StringArray &prefabs){
#ifdef JSON_DISABLE_PREFAB_IMPORT
	UE_LOG(JsonLog, Warning, TEXT("Prefab import is currently disabled"));
	return;
#endif

	FScopedSlowTask progress(prefabs.Num(), LOCTEXT("Importing prefabs", "Importing prefabs"));
	progress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Import prefabs"));
	int32 objId = 0;
	for(auto curFilename: prefabs){
		auto obj = loadExternResourceFromFile(curFilename);
		auto curId = objId;
		objId++;
		if (!obj.IsValid())
			continue;

		auto prefab = JsonPrefabData(obj);

		importPrefab(prefab);

		progress.EnterProgressFrame(1.0f);
	}
}
