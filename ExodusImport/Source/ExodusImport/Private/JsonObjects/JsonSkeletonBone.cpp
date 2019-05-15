#include "JsonImportPrivatePCH.h"
#include "JsonSkeletonBone.h"
#include "JsonScene.h"
#include "macros.h"

void JsonSkeletonBone::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, parentId);
	JSON_GET_VAR(data, world);
	JSON_GET_VAR(data, local);
	JSON_GET_VAR(data, rootRelative);
}
