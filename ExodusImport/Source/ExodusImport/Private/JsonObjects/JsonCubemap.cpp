#include "JsonImportPrivatePCH.h"
#include "JsonCubemap.h"
#include "macros.h"

//#define JSON_ENABLE_VALUE_LOGGING

using namespace JsonObjects;

void JsonCubemap::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR_LOG(data, id);
	JSON_GET_VAR_LOG(data, name);
	JSON_GET_VAR(data, exportPath);
	JSON_GET_VAR(data, assetPath);
	JSON_GET_VAR(data, rawPath);
	JSON_GET_VAR(data, needConversion);
	JSON_GET_VAR(data, isHdr);
	JSON_GET_VAR(data, format);

	getJsonObj(data, texParams, "texParams");
	getJsonObj(data, texImportParams, "texImportParams");
}
	