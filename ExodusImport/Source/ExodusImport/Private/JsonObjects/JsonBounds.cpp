#include "JsonImportPrivatePCH.h"
#include "JsonBounds.h"
#include "macros.h"

void JsonBounds::load(JsonObjPtr jsonData){
	using namespace JsonObjects;
	JSON_GET_PARAM(jsonData, center, getVector);
	JSON_GET_PARAM(jsonData, size, getVector);
}

JsonBounds::JsonBounds(JsonObjPtr jsonData){
	load(jsonData);
}

