#pragma once

#include "getters.h"

#define JSON_GET_AUTO_PARAM2_LOG(obj, varName, paramName, op) auto varName = op(obj, #paramName); logValue(#paramName, varName);
#define JSON_GET_AUTO_PARAM_LOG(obj, name, op) auto name = op(obj, #name); logValue(#name, name);

#define JSON_GET_AUTO_PARAM2_NOLOG(obj, varName, paramName, op) auto varName = op(obj, #paramName);
#define JSON_GET_AUTO_PARAM_NOLOG(obj, name, op) auto name = op(obj, #name);

#define JSON_GET_PARAM2_LOG(obj, varName, paramName, op) varName = op(obj, #paramName); logValue(#paramName, varName);
#define JSON_GET_PARAM_LOG(obj, name, op) name = op(obj, #name); logValue(#name, name);

#define JSON_GET_PARAM2_NOLOG(obj, varName, paramName, op) varName = op(obj, #paramName);
#define JSON_GET_PARAM_NOLOG(obj, name, op) name = op(obj, #name);

#define JSON_GET_VAR_LOG(obj, name) JsonObjects::getJsonValue(name, obj, #name); logValue(#name, name);
#define JSON_GET_VAR2_LOG(obj, name, paramName) JsonObjects::getJsonValue(name, obj, #paramName); logValue(#paramName, name);

#define JSON_GET_VAR_NOLOG(obj, name) JsonObjects::getJsonValue(name, obj, #name);
#define JSON_GET_VAR2_NOLOG(obj, name, paramName) JsonObjects::getJsonValue(name, obj, #paramName);

#ifdef JSON_ENABLE_VALUE_LOGGING

#define JSON_GET_VAR(obj, name) JSON_GET_VAR_LOG(obj, name)
#define JSON_GET_VAR2(obj, name, paramName) JSON_GET_VAR2_LOG(obj, name, paramName) 

#define JSON_GET_AUTO_PARAM2(obj, varName, paramName, op) JSON_GET_AUTO_PARAM2_LOG(obj, varName, paramName, op)(#paramName, varName);
#define JSON_GET_AUTO_PARAM(obj, name, op) JSON_GET_AUTO_PARAM_LOG(obj, name, op)

#define JSON_GET_PARAM2(obj, varName, paramName, op) JSON_GET_PARAM2_LOG(obj, varName, paramName, op)
#define JSON_GET_PARAM(obj, name, op) JSON_GET_PARAM_LOG(obj, name, op)

#else

#define JSON_GET_AUTO_PARAM2(obj, varName, paramName, op) JSON_GET_AUTO_PARAM2_NOLOG(obj, varName, paramName, op)(#paramName, varName);
#define JSON_GET_AUTO_PARAM(obj, name, op) JSON_GET_AUTO_PARAM_NOLOG(obj, name, op)

#define JSON_GET_VAR(obj, name) JSON_GET_VAR_NOLOG(obj, name)
#define JSON_GET_VAR2(obj, name, paramName) JSON_GET_VAR2_NOLOG(obj, name, paramName) 

#define JSON_GET_PARAM2(obj, varName, paramName, op) JSON_GET_PARAM2_NOLOG(obj, varName, paramName, op)
#define JSON_GET_PARAM(obj, name, op) JSON_GET_PARAM_NOLOG(obj, name, op)

#endif

#define JSON_GET_OBJ(data, objName) JsonObjects::getJsonObj(data, objName, #objName);
#define JSON_GET_ARRAY(data, objName) JsonObjects::getJsonObjArray(data, objName, #objName);
