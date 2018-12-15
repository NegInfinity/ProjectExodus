#include "JsonImportPrivatePCH.h"
#include "JsonSpriteMetaData.h"
#include "macros.h"

//#define JSON_ENABLE_VALUE_LOGGING

using namespace JsonObjects;

void JsonSpriteMetaData::load(JsonObjPtr jsonData){
	JSON_GET_VAR(jsonData, name);
	JSON_GET_VAR(jsonData, pivot);
	JSON_GET_VAR(jsonData, border);
	JSON_GET_VAR(jsonData, alignment);
	getJsonObj(jsonData, rect, "rect");
}

