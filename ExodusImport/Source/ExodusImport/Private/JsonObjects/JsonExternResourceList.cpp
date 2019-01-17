#include "JsonImportPrivatePCH.h"
#include "JsonExternResourceList.h"
#include "macros.h"

#define JSON_ENABLE_VALUE_LOGGING

using namespace JsonObjects;

void JsonExternResourceList::load(JsonObjPtr data){
	JSON_GET_VAR(data, scenes);
	JSON_GET_VAR(data, materials);
	JSON_GET_VAR(data, skeletons);
	JSON_GET_VAR(data, meshes);
	JSON_GET_VAR(data, textures);
	JSON_GET_VAR(data, prefabs);
	JSON_GET_VAR(data, terrains);
	JSON_GET_VAR(data, cubemaps);
	JSON_GET_VAR(data, audioClips);
	JSON_GET_VAR(data, animationClips);
	JSON_GET_VAR(data, animatorControllers);
	JSON_GET_VAR(data, resources);
}

