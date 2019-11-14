#include "JsonImportPrivatePCH.h"
#include "ImportContext.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "JsonObjects/utilities.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Engine/World.h"
#include "UnrealUtilities.h"


void ImportContext::registerDelayedAnimController(JsonId skelId, JsonId controllerId){
	delayedAnimControllers.Add(AnimControllerIdKey(skelId, controllerId));
}

void ImportContext::registerAnimatorForPostProcessing(const JsonGameObject &jsonObj){
	if (!postProcessAnimatorObjects.Contains(jsonObj.id))
		postProcessAnimatorObjects.Add(jsonObj.id);
}

const JsonGameObject* ImportContext::findJsonObject(JsonId id) const{
	check(srcObjects != nullptr);
	if ((id >= 0) && (id < srcObjects->Num()))
		return &(*srcObjects)[id];

	return nullptr;
}

const JsonRigidbody* ImportContext::locateRigidbody(const JsonGameObject &srcGameObj) const{
	using namespace JsonObjects;

	const JsonGameObject *currentObject = &srcGameObj;
	while (currentObject){
		if (currentObject->rigidbodies.Num() > 0){
			return &currentObject->rigidbodies[0];
		}
		if (!isValidId(currentObject->parentId))
			break;

		if (!srcObjects)
			break;

		currentObject = findJsonObject(currentObject->parentId);
	}
	return nullptr;
}

void ImportContext::registerGameObject(const JsonGameObject &gameObj, ImportedObject imported){
	check(JsonObjects::isValidId(gameObj.id));
	check(imported.isValid());
	/*if (importedObjects.Contains(gameObj.id))
		UE_LOG(JsonLog, Warning, TEXT("Object re-registration for id %s"), gameObj.id);
	*/
	importedObjects.Add(gameObj.id, imported);
}

const ImportedObject* ImportContext::findImportedObject(JsonId id) const{
	return importedObjects.Find(id);
}

ImportedObject* ImportContext::findImportedObject(JsonId id){
	return importedObjects.Find(id);
}

const FString* ImportContext::findFolderPath(JsonId id) const{
	return objectFolderPaths.Find(id);
}

FString ImportContext::processFolderPath(const JsonGameObject &jsonGameObj){
	FString folderPath;
	FString childFolderPath = jsonGameObj.ueName;
	if (jsonGameObj.parentId >= 0){
		const FString* found = objectFolderPaths.Find(jsonGameObj.parentId);
		if (found){
			folderPath = *found;
			childFolderPath = folderPath + "/" + jsonGameObj.ueName;
		}
		else{
			UE_LOG(JsonLog, Warning, TEXT("Object parent not found, folder path may be invalid"));
		}
	}

	//UE_LOG(JsonLog, Log, TEXT("Folder path for object: %d: %s"), jsonGameObj.id, *folderPath);
	objectFolderPaths.Add(jsonGameObj.id, childFolderPath);
	return folderPath;
}

UObject* ImportContext::findSuitableOuter(const JsonGameObject &jsonObj) const{
	using namespace JsonObjects;

	JsonId parentId = jsonObj.parentId;
	UObject* result = nullptr;
	while (isValidId(parentId)){
		auto foundImported = importedObjects.Find(parentId);
		if (foundImported){
			auto rootActor = foundImported->findRootActor();
			if (rootActor)
				return rootActor;
		}
		auto foundJson = findJsonObject(parentId);
		if (!foundJson)
			break;
		parentId = foundJson->parentId;
	}
	return result;
}

bool ImportContext::isCompoundRigidbodyRootCollider(const JsonGameObject &gameObj) const{
	using namespace JsonObjects;

	if (!gameObj.hasColliders())
		return false;
	if (gameObj.hasRigidbody())
		return true;
	int parentId = gameObj.parentId;
	auto rigBody = locateRigidbody(gameObj);
	if (!rigBody)
		return false;

	while (isValidId(parentId)){
		auto parentJson = findJsonObject(parentId);
		if (!parentJson){
			UE_LOG(JsonLog, Warning, TEXT("Broken object chain at %s(\"%s\", %d)"), *gameObj.name, *gameObj.scenePath, gameObj.id);
			return false;//broken object chain. Report?
		}
		if (parentJson->hasColliders())
			return false;//we are not at compound object root, because parent has collides of its own.
		if (parentJson->hasRigidbody())
			return true;//parent has rigibody and no colliders. We're possible compound root.
		parentId = parentJson->parentId;		
	}
	check(false);//somehow we found neither rigidbody nor colliders? This is an illegal scene graph and likely an error
	return false;
}

AActor* ImportContext::createBlankActor(const JsonGameObject &gameObj, bool createMissingRootComponent) const{
	using namespace UnrealUtilities;
	check(world);
	FTransform transform;
	transform.SetFromMatrix(gameObj.ueWorldMatrix);

	AActor *blankActor = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	USceneComponent *rootComponent = nullptr;
	if (createMissingRootComponent){
		rootComponent = NewObject<USceneComponent>(blankActor);
		rootComponent->SetWorldTransform(transform);
	}

	if (rootComponent){
		blankActor->SetRootComponent(rootComponent);
		rootComponent->SetMobility(gameObj.getUnrealMobility());
	}
	blankActor->SetActorLabel(gameObj.ueName, true);
	return blankActor;
}

ImportedObject ImportContext::createBlankActor(const JsonGameObject &gameObj, USceneComponent *rootComponent, bool changeOwnership, bool createMissingRootComponent) const{
	using namespace UnrealUtilities;
	check(world);
	FTransform transform;
	transform.SetFromMatrix(gameObj.ueWorldMatrix);

	AActor *blankActor = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	if (!rootComponent){
		if (createMissingRootComponent){
			rootComponent = NewObject<USceneComponent>(blankActor);
			rootComponent->SetWorldTransform(transform);
		}
	}
	else{
		if (changeOwnership){
			changeOwnerRecursively(rootComponent, blankActor);//Due to the way it works, this is not enough to fix ownership issues when importing prefabs
		}
	}

	if (rootComponent){
		blankActor->SetRootComponent(rootComponent);
		rootComponent->SetMobility(gameObj.getUnrealMobility());
	}
	blankActor->SetActorLabel(gameObj.ueName, true);
	ImportedObject importedObject(blankActor);
	return importedObject;
}

ImportedObject ImportContext::createBlankNode(const JsonGameObject &gameObj, bool createActor, bool createMissingRootComponent, std::function<UObject*()> outerGetter) const{
	if (createActor)
		return createBlankActor(gameObj, nullptr, true, createMissingRootComponent);

	FTransform transform;
	transform.SetFromMatrix(gameObj.ueWorldMatrix);
	UObject *outerPtr = nullptr;
	if (outerGetter){
		outerPtr = outerGetter();
		check(outerPtr != nullptr);
	}
	else{
		outerPtr = GetTransientPackage();
	}

	auto *rootComponent = NewObject<USceneComponent>(outerPtr);
	rootComponent->SetWorldTransform(transform);
	rootComponent->SetMobility(gameObj.getUnrealMobility());

	//auto nameGuid = FGuid::NewGuid();
	auto nodeName = FString::Printf(TEXT("%s_node(%d_%llu)"), *gameObj.ueName, gameObj.id, getUniqueUint());
	rootComponent->Rename(*nodeName);

	ImportedObject importedObject(rootComponent);
	return importedObject;
}


ImportContext::ImportContext(UWorld *world_, bool editorMode_, const JsonScene *scene_)
:srcObjects(nullptr), world(world_), editorMode(editorMode_){
	check(scene_ != nullptr);
	srcObjects = &scene_->objects;
}

ImportContext::ImportContext(UWorld *world_, bool editorMode_, const TArray<JsonGameObject> *objects_)
:srcObjects(objects_), world(world_), editorMode(editorMode_){
}


void ImportContext::clear(){
	objectFolderPaths.Empty();
	importedObjects.Empty();

	delayedAnimControllers.Empty();
}

uint64 ImportContext::getUniqueUint() const{
	return uniqueInt++;//Should I switch to guids?
}
