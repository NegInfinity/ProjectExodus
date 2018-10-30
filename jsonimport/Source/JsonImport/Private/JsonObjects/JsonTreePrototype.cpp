#include "JsonImportPrivatePCH.h"
#include "JsonTreePrototype.h"
#include "macros.h"

using namespace JsonObjects;

JsonTreePrototype::JsonTreePrototype(JsonObjPtr data){
	load(data);
}

void JsonTreePrototype::load(JsonObjPtr data){
	JSON_GET_VAR(data, prefabId);
	JSON_GET_VAR(data, prefabObjectId);
	JSON_GET_VAR(data, meshId);
	JSON_GET_VAR(data, materials);
}
