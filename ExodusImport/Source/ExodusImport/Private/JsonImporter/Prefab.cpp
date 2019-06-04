#include "JsonImportPrivatePCH.h"
#include "CoreMinimal.h"
#include "JsonTypes.h"
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