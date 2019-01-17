#include "JsonImportPrivatePCH.h"
#include "JsonTreeInstance.h"
#include "macros.h"

//#define JSON_ENABLE_VALUE_LOGGING

JsonTreeInstance::JsonTreeInstance(JsonObjPtr data){
	load(data);
}

void JsonTreeInstance::load(JsonObjPtr data){
	JSON_GET_VAR(data, color);
	JSON_GET_VAR(data, heightScale);
	JSON_GET_VAR(data, lightmapColor);
	JSON_GET_VAR(data, position);
	JSON_GET_VAR(data, rotation);
	JSON_GET_VAR(data, prototypeIndex);
	JSON_GET_VAR(data, widthScale);
}
