#include "JsonImportPrivatePCH.h"
#include "ImportWorkData.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "JsonObjects/utilities.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Engine/World.h"


void ImportWorkData::registerDelayedAnimController(JsonId skelId, JsonId controllerId){
	delayedAnimControllers.Add(AnimControllerIdKey(skelId, controllerId));
}

void ImportWorkData::registerAnimatorForPostProcessing(const JsonGameObject &jsonObj){
	if (!postProcessAnimatorObjects.Contains(jsonObj.id))
		postProcessAnimatorObjects.Add(jsonObj.id);
}

const JsonGameObject* ImportWorkData::findJsonObject(JsonId id) const{
	//check(srcScene != nullptr);
	check(srcObjects != nullptr);
	//if ((id >= 0) && (id < srcScene->objects.Num()))
	if ((id >= 0) && (id < srcObjects->Num()))
		return &(*srcObjects)[id];
		//return &srcScene->objects[id];

	return nullptr;
}

const JsonRigidbody* ImportWorkData::locateRigidbody(const JsonGameObject &srcGameObj) const{
	using namespace JsonObjects;

	const JsonGameObject *currentObject = &srcGameObj;
	while (currentObject){
		if (currentObject->rigidbodies.Num() > 0){
			return &currentObject->rigidbodies[0];
		}
		if (!isValidId(currentObject->parentId))
			break;

		//if (!srcScene)
		if (!srcObjects)
			break;

		currentObject = findJsonObject(currentObject->parentId);
	}
	return nullptr;
}

void ImportWorkData::registerGameObject(const JsonGameObject &gameObj, ImportedObject imported){
	check(JsonObjects::isValidId(gameObj.id));
	check(imported.isValid());
	/*if (importedObjects.Contains(gameObj.id))
		UE_LOG(JsonLog, Warning, TEXT("Object re-registration for id %s"), gameObj.id);
	*/
	importedObjects.Add(gameObj.id, imported);
}

const ImportedObject* ImportWorkData::findImportedObject(JsonId id) const{
	return importedObjects.Find(id);
}

ImportedObject* ImportWorkData::findImportedObject(JsonId id){
	return importedObjects.Find(id);
}

const FString* ImportWorkData::findFolderPath(JsonId id) const{
	return objectFolderPaths.Find(id);
}

FString ImportWorkData::processFolderPath(const JsonGameObject &jsonGameObj){
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

UObject* ImportWorkData::findSuitableOuter(const JsonGameObject &jsonObj) const{
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

bool ImportWorkData::isCompoundRigidbodyRootCollider(const JsonGameObject &gameObj) const{
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

ImportedObject ImportWorkData::createBlankActor(const JsonGameObject &jsonGameObj) const{
	check(world);
	FTransform transform;
	transform.SetFromMatrix(jsonGameObj.ueWorldMatrix);

	AActor *blankActor = world->SpawnActor<AActor>(AActor::StaticClass(), transform);
	auto *rootComponent = NewObject<USceneComponent>(blankActor);
	rootComponent->SetWorldTransform(transform);
	blankActor->SetRootComponent(rootComponent);
	blankActor->SetActorLabel(jsonGameObj.ueName, true);
	rootComponent->SetMobility(jsonGameObj.getUnrealMobility());
	ImportedObject importedObject(blankActor);
	return importedObject;
}

ImportedObject ImportWorkData::createBlankNode(const JsonGameObject &gameObj, bool createActor) const{
	if (createActor)
		return createBlankActor(gameObj);

	FTransform transform;
	transform.SetFromMatrix(gameObj.ueWorldMatrix);
	auto *rootComponent = NewObject<USceneComponent>();
	rootComponent->SetWorldTransform(transform);
	rootComponent->SetMobility(gameObj.getUnrealMobility());

	//auto nameGuid = FGuid::NewGuid();
	auto nodeName = FString::Printf(TEXT("%s_node(%d_%llu)"), *gameObj.ueName, gameObj.id, getUniqueUint());
	rootComponent->Rename(*nodeName);

	ImportedObject importedObject(rootComponent);
	return importedObject;
}


ImportWorkData::ImportWorkData(UWorld *world_, bool editorMode_, const JsonScene *scene_)
:srcObjects(nullptr), world(world_), editorMode(editorMode_){
	check(scene_ != nullptr);
	srcObjects = &scene_->objects;
}

ImportWorkData::ImportWorkData(UWorld *world_, bool editorMode_, const TArray<JsonGameObject> *objects_)
:srcObjects(objects_), world(world_), editorMode(editorMode_){
}


void ImportWorkData::clear(){
	objectFolderPaths.Empty();
	importedObjects.Empty();

	delayedAnimControllers.Empty();
}

uint64 ImportWorkData::getUniqueUint() const{
	//static uint64 val = 0;
	//return val++;
	return uniqueInt++;//Should I switch to guids?
}
