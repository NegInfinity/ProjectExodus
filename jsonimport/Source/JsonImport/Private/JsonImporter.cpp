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

#define LOCTEXT_NAMESPACE "FJsonImportModule"

#define JSON_DISABLE_PREFAB_IMPORT

using namespace JsonObjects;
using namespace UnrealUtilities;
//using namespace MaterialTools;

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
	AActor *rootActor = 0;
	if (workData.objectActors.Contains(0)){
		rootActor = workData.objectActors[0];
	}
	else{
		FTransform firstObjectTransform;
		firstObjectTransform.SetFromMatrix(firstObject.ueWorldMatrix);
		rootActor = createActor<AActor>(workData, firstObjectTransform, TEXT("AActor"));
	}

	UE_LOG(JsonLogPrefab, Log, TEXT("Attaching actors. %d actors present"), workData.rootActors.Num());

	TArray<AActor*> prefabActors = workData.rootActors;
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
}

void JsonImporter::importPrefabs(const JsonValPtrs *prefabs){
#ifdef JSON_DISABLE_PREFAB_IMPORT
	UE_LOG(JsonLog, Warning, TEXT("Prefab import is currently disabled"));
	return;
#endif

	if (!prefabs)
		return;
	FScopedSlowTask progress(prefabs->Num(), LOCTEXT("Importing prefabs", "Importing prefabs"));
	progress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Import prefabs"));
	int32 objId = 0;
	for(auto cur: *prefabs){
		auto obj = cur->AsObject();
		auto curId = objId;
		objId++;
		if (!obj.IsValid())
			continue;

		auto prefab = JsonPrefabData(obj);

		importPrefab(prefab);

		//importObject(obj, objId, importData);
		progress.EnterProgressFrame(1.0f);

		//break
	}
}


void JsonImporter::importResources(JsonObjPtr jsonData){
	const JsonValPtrs *resources = 0, *textures = 0, *materials = 0, *meshes = 0, *prefabs = 0;

	loadArray(jsonData, resources, TEXT("resources"), TEXT("Resources"));
	loadArray(jsonData, textures, TEXT("textures"), TEXT("Textures"));
	loadArray(jsonData, materials, TEXT("materials"), TEXT("Materials"));
	loadArray(jsonData, meshes, TEXT("meshes"), TEXT("Meshes"));
	loadArray(jsonData, prefabs, TEXT("prefabs"), TEXT("Prefabs"));

	assetCommonPath = findCommonPath(resources);

	loadTextures(textures);
	loadMaterials(materials);
	loadMeshes(meshes);

	importPrefabs(prefabs);
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
