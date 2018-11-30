#include "JsonImportPrivatePCH.h"
#include "JsonScene.h"
#include "macros.h"

using namespace JsonObjects;

void JsonSkeletonBone::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, parent);
	JSON_GET_VAR(data, pose);
	JSON_GET_VAR(data, invPose);
}
