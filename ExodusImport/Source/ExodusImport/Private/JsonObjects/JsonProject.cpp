#include "JsonImportPrivatePCH.h"
#include "JsonProject.h"
#include "macros.h"

void JsonProject::load(JsonObjPtr data){
	using namespace JsonObjects;
	getJsonObj(data, config, "config");
	getJsonObj(data, externResources, "externResources");
}
