#include "JsonImportPrivatePCH.h"
#include "macros.h"

using namespace JsonObjects;

void JsonProject::load(JsonObjPtr data){
	getJsonObj(data, config, "config");
	getJsonObj(data, externResources, "externResources");
}
