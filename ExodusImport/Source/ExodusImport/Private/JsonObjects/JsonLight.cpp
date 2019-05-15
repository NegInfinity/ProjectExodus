#include "JsonImportPrivatePCH.h"
#include "JsonLight.h"
#include "macros.h"

void JsonLight::load(JsonObjPtr jsonData){
	using namespace JsonObjects;
	JSON_GET_PARAM(jsonData, range, getFloat);
	JSON_GET_PARAM(jsonData, spotAngle, getFloat);
	JSON_GET_PARAM2(jsonData, lightType, type, getString)
	JSON_GET_PARAM(jsonData, shadowStrength, getFloat);
	JSON_GET_PARAM(jsonData, intensity, getFloat);
	JSON_GET_PARAM(jsonData, bounceIntensity, getFloat);
	JSON_GET_PARAM(jsonData, color, getColor);
	JSON_GET_PARAM(jsonData, renderMode, getString);
	JSON_GET_PARAM(jsonData, shadows, getString);
	castsShadows = shadows != "Off";
}

JsonLight::JsonLight(JsonObjPtr jsonData){
	load(jsonData);
}

