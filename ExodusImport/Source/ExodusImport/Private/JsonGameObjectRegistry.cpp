#include "JsonImportPrivatePCH.h"
#include "JsonGameObjectRegistry.h"
#include "JsonLog.h"

bool JsonGameObjectRegistry::isValidIndex(int index) const{
	return (index >= 0) && (index < objects.Num());
}

void JsonGameObjectRegistry::clear(){
	objects.Empty();
	instanceIdMap.clear();
}

void JsonGameObjectRegistry::registerInstanceId(const JsonGameObject &obj){
	auto instId = obj.instanceId;
	auto jsonId = obj.id;
	if (!instanceIdMap.registerId(instId, jsonId)){
		UE_LOG(JsonLog, Warning, TEXT("Duplicate Id found while registering instanceId %d for object %d(%s)."),
			instId, jsonId, *obj.name);
	}
}

//This function must be called in strictly sequential fashion!
void JsonGameObjectRegistry::addObject(const JsonGameObject &newObj){
	check(newObj.id == size());
	objects.Add(newObj);
	registerInstanceId(newObj);
}

const JsonGameObject* JsonGameObjectRegistry::findByInstanceId(InstanceId instId) const{
	auto foundId = instanceIdMap.find(instId);
	if (!foundId)
		return nullptr;
	return findByIndex(*foundId);
}

const JsonGameObject* JsonGameObjectRegistry::findByIndex(int index) const{
	if (!isValidIndex(index))
		return nullptr;
	return &objects[index];
}

const JsonGameObject& JsonGameObjectRegistry::operator[](int index) const{
	check(isValidIndex(index));
	return objects[index];
}

void JsonGameObjectRegistry::rebuildInstanceIdMap(){
	instanceIdMap.clear();
	for (const auto& cur : objects){
		registerInstanceId(cur);
	}
}

const JsonGameObject* JsonGameObjectRegistry::resolveObjectReference(const JsonObjectReference &ref) const{
	if (ref.isNull)
		return nullptr;
	auto found = instanceIdMap.find(ref.instanceId);
	if (!found)
		return nullptr;
	if (!isValidIndex(*found))
		return nullptr;
	return &objects[*found];
}

void JsonGameObjectRegistry::buildFrom(const JsonGameObjectArray &objects){
	clear();
	for (const auto& cur : objects)
		addObject(cur);
}
