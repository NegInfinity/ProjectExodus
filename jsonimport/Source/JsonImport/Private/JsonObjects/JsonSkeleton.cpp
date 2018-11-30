#include "JsonImportPrivatePCH.h"
#include "JsonSkeleton.h"
#include "macros.h"

using namespace JsonObjects;

void JsonSkeleton::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, name);

	getJsonObjArray(data, bones, "bones");
}
