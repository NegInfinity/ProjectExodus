#include "JsonImportPrivatePCH.h"
#include "SkeletalMeshComponentBuilder.h"
#include "UnrealUtilities.h"
#include "JsonImporter.h"
#include "Classes/Animation/SkeletalMeshActor.h"
#include "Classes/Components/SkeletalMeshComponent.h"

void SkeletalMeshComponentBuilder::processSkinMeshes(ImportWorkData &workData, const JsonGameObject &gameObj, 
		ImportedObject *parentObject, const FString &folderPath, ImportedObjectArray *createdObjects, JsonImporter *importer){
	using namespace UnrealUtilities;
	check(importer != nullptr);
	for(const auto &jsonSkin: gameObj.skinRenderers){
		if (!gameObj.activeInHierarchy)//Temporary hack to debug
			continue;
		auto skinMesh = processSkinRenderer(workData, gameObj, jsonSkin, parentObject, folderPath, importer);
		registerImportedObject(createdObjects, skinMesh);
	}
}

ImportedObject SkeletalMeshComponentBuilder::processSkinRenderer(ImportWorkData &workData, const JsonGameObject &jsonGameObj, 
		const JsonSkinRenderer &skinRend, ImportedObject *parentObject, const FString &folderPath, JsonImporter *importer){
	using namespace UnrealUtilities;
	check(importer != nullptr);

	UE_LOG(JsonLog, Log, TEXT("Importing skin mesh %d for object %s"), skinRend.meshId.toIndex(), *jsonGameObj.name);
	//if (skinRend.meshId < 0)
	if (!skinRend.meshId.isValid())
		return ImportedObject();

	auto foundMeshPath = importer->getSkinMeshIdMap().Find(skinRend.meshId);//skinMeshIdMap.Find(skinRend.meshId);
	if (!foundMeshPath){
		UE_LOG(JsonLog, Log, TEXT("Could not locate skin mesh %d for object %s"), skinRend.meshId.toIndex(), *jsonGameObj.name);
		return ImportedObject();
	}

	auto *skelMesh = importer->loadSkeletalMeshById(skinRend.meshId);
	if (!skelMesh){
		UE_LOG(JsonLog, Error, TEXT("Coudl not load skinMesh %d on object %d(%s)"), skinRend.meshId.toIndex(), jsonGameObj.id, *jsonGameObj.name);
		return ImportedObject();
	}


	/*
	This is great.

	Looks like there's major discrepancy in how components work in unity and unreal engine.

	Unity skinned mesh acts as BOTH PoseableMesh and SkeletalMesh, meaning you can move individual bones around while they're being animated.
	*/
	FActorSpawnParameters spawnParams;
	FTransform transform;
	transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

	ASkeletalMeshActor *meshActor = workData.world->SpawnActor<ASkeletalMeshActor>(ASkeletalMeshActor::StaticClass(), transform, spawnParams);
	if (!meshActor){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't spawn skeletal actor"));
		return ImportedObject();
	}

	meshActor->SetActorLabel(jsonGameObj.ueName, true);

	USkeletalMeshComponent *meshComponent = meshActor->GetSkeletalMeshComponent();

	meshComponent->SetSkeletalMesh(skelMesh, true);

	const auto& materials = skinRend.materials;
	if (materials.Num() > 0){
		for(int i = 0; i < materials.Num(); i++){
			auto matId = materials[i];

			auto material = importer->loadMaterialInterface(matId);
			meshComponent->SetMaterial(i, material);
		}
	}

	ImportedObject importedObject(meshActor);
	//workData.importedObjects.Add(jsonGameObj.id, importedObject);
	workData.registerGameObject(jsonGameObj, importedObject);
	setObjectHierarchy(importedObject, parentObject, folderPath, workData, jsonGameObj);
	return importedObject;
}