#include "JsonImportPrivatePCH.h"
#include "ImportWorkData.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "JsonObjects/utilities.h"

void ImportWorkData::registerDelayedAnimController(JsonId skelId, JsonId controllerId){
	delayedAnimControllers.Add(AnimControllerIdKey(skelId, controllerId));
}

void ImportWorkData::registerAnimatorForPostProcessing(const JsonGameObject &jsonObj){
	if (!postProcessAnimatorObjects.Contains(jsonObj.id))
		postProcessAnimatorObjects.Add(jsonObj.id);
}

const JsonGameObject* ImportWorkData::findJsonObject(JsonId id) const{
	check(scene != nullptr);
	if ((id >= 0) && (id < scene->objects.Num()))
		return &scene->objects[id];

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

		if (!scene)
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
