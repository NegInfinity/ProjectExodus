#include "JsonImportPrivatePCH.h"
#include "JsonPrefabData.h"
#include "macros.h"

void JsonPrefabData::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, name, getString);
	JSON_GET_PARAM(jsonData, path, getString);
	JSON_GET_PARAM(jsonData, guid, getString);
	JSON_GET_PARAM(jsonData, prefabType, getString);

	objects.Empty();
	getJsonObjArray(jsonData, objects, "objects");
	//objects
}

JsonPrefabData::JsonPrefabData(JsonObjPtr jsonData){
	load(jsonData);
}

