#include "JsonImportPrivatePCH.h"
#include "JsonRenderer.h"
#include "macros.h"

void JsonRenderer::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, lightmapIndex, getInt);
	JSON_GET_PARAM(jsonData, shadowCastingMode, getString);
	JSON_GET_PARAM(jsonData, receiveShadows, getBool);

	JSON_GET_PARAM(jsonData, materials, getIntArray);
}

