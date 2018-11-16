#include "JsonImportPrivatePCH.h"
#include "JsonCubemap.h"
#include "macros.h"

using namespace JsonObjects;

void JsonCubemap::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, exportPath);
	JSON_GET_VAR(data, assetPath);
	JSON_GET_VAR(data, rawPath);
	JSON_GET_VAR(data, needConversion);
	JSON_GET_VAR(data, isHdr);
	JSON_GET_VAR(data, format);

	getJsonObj(data, texParams, "texParams");
	getJsonObj(data, texImportParams, "texImportParams");
}
	