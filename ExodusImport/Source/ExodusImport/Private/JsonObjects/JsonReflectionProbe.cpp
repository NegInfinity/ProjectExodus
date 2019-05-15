#include "JsonImportPrivatePCH.h"
#include "JsonReflectionProbe.h"
#include "macros.h"

void JsonReflectionProbe::load(JsonObjPtr jsonData){
	using namespace JsonObjects;

	JSON_GET_PARAM(jsonData, backgroundColor, getColor);
	JSON_GET_PARAM(jsonData, blendDistance, getFloat);
	JSON_GET_PARAM(jsonData, boxProjection, getBool);
	JSON_GET_PARAM(jsonData, center, getVector);
	JSON_GET_PARAM(jsonData, size, getVector);
	JSON_GET_PARAM(jsonData, clearType, getString);
	JSON_GET_PARAM(jsonData, cullingMask, getInt);
	JSON_GET_PARAM(jsonData, hdr, getBool);
	JSON_GET_PARAM(jsonData, intensity, getFloat);
	JSON_GET_PARAM(jsonData, nearClipPlane, getFloat);
	JSON_GET_PARAM(jsonData, farClipPlane, getFloat);
	JSON_GET_PARAM(jsonData, resolution, getInt);
	JSON_GET_PARAM(jsonData, mode, getString);
	JSON_GET_PARAM(jsonData, refreshMode, getString);

	JSON_GET_PARAM(jsonData, customCubemapId, getInt);
	JSON_GET_PARAM(jsonData, customTex2dId, getInt);
}

JsonReflectionProbe::JsonReflectionProbe(JsonObjPtr jsonData){
	load(jsonData);
}

