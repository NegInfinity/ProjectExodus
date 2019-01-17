#include "JsonImportPrivatePCH.h"
#include "JsonRect.h"
#include "macros.h"

void JsonRect::load(JsonObjPtr data){
	JSON_GET_VAR2(data, minPoint, min);
	JSON_GET_VAR2(data, maxPoint, max);
}
