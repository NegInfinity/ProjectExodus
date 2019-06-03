#include "JsonImportPrivatePCH.h"
#include "JsonImporter.h"

#include "Classes/Engine/Blueprint.h"
#include "AssetRegistry/Public/AssetRegistryModule.h"
#include "UnrealEd/Public/Kismet2/KismetEditorUtilities.h"
#include "UnrealEd/Public/Editor.h"
#include "UnrealEd/Public/Toolkits/AssetEditorManager.h"
#include "Internationalization/Internationalization.h"

#include "builders/PrefabBuilder.h"
#include "LocTextNamespace.h"

#define LOCTEXT_NAMESPACE LOCTEXT_NAMESPACE_NAME

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

		PrefabBuilder builder;
		auto prefab = JsonPrefabData(obj);

		builder.importPrefab(prefab, this);
		//importPrefab(prefab);

		progress.EnterProgressFrame(1.0f);
	}
}

#undef LOCTEXT_NAMESPACE
