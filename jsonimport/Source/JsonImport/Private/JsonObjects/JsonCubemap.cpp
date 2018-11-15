#include "JsonImportPrivatePCH.h"
#include "JsonCubemap.h"
#include "macros.h"

using namespace JsonObjects;

void JsonCubemap::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, exportPath);
	JSON_GET_VAR(data, assetPath);
	JSON_GET_VAR(data, needConversion);

	getJsonObj(data, texParams, "textureParams");
	getJsonObj(data, texImportParams, "textureImportParams");
}
	