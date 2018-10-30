#include "JsonImportPrivatePCH.h"
#include "JsonSplatPrototype.h"
#include "macros.h"

void JsonSplatPrototype::load(JsonObjPtr data){
	JSON_GET_VAR(data, textureId);
	JSON_GET_VAR(data, normalMapId);
			
	JSON_GET_VAR(data, metallic);
	JSON_GET_VAR(data, smoothness);
	JSON_GET_VAR(data, specular);
	JSON_GET_VAR(data, tileOffset);
	JSON_GET_VAR(data, tileSize);
}

JsonSplatPrototype::JsonSplatPrototype(JsonObjPtr data){
	load(data);
}
