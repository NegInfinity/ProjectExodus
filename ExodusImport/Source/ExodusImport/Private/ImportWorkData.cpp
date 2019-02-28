#include "JsonImportPrivatePCH.h"
#include "ImportWorkData.h"
#include "Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

void ImportWorkData::registerObject(const ImportedObject &object, AActor *parent){
	if (!storeActors)
		return;
	if (!parent)
		rootObjects.Add(object);
	else
		childObjects.Add(object);
	allObjects.Add(object);
}

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

