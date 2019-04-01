#pragma once
#include "JsonObjects.h"

class JsonScene{
public:
	FString name;
	FString path;
	int buildIndex = -1;
	using InstanceId = int;
	TArray<JsonGameObject> objects;

	TMap<InstanceId, JsonId> gameObjectInstanceIdMap;

	bool containsTerrain() const;

	const JsonGameObject* findJsonObject(JsonId id) const;
	const JsonGameObject* findJsonObjectByInstId(InstanceId instId) const;

	void load(JsonObjPtr data);
	JsonScene() = default;
	JsonScene(JsonObjPtr data){
		load(data);
	}
protected:
	void buildInstanceIdMap();
};
