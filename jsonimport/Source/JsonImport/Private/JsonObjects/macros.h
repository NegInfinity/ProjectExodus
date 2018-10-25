#pragma once

#include "getters.h"

#define JSON_GET_AUTO_PARAM2(obj, varName, paramName, op) auto varName = op(obj, #paramName); logValue(#paramName, varName);
#define JSON_GET_AUTO_PARAM(obj, name, op) auto name = op(obj, #name); logValue(#name, name);

#define JSON_GET_PARAM2(obj, varName, paramName, op) varName = op(obj, #paramName); logValue(#paramName, varName);
#define JSON_GET_PARAM(obj, name, op) name = op(obj, #name); logValue(#name, name);

#define JSON_GET_VAR(obj, name) JsonObjects::getJsonValue(name, obj, #name); logValue(#name, name);
#define JSON_GET_VAR2(obj, name, paramName) JsonOBjects::getJsonValue(name, obj, #paramName); logValue(#paramName, name);

