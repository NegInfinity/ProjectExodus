#include "JsonImportPrivatePCH.h"
#include "JsonScene.h"
#include "macros.h"

using namespace JsonObjects;

void JsonScene::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, buildIndex);

	getJsonObjArray(data, objects, "objects");
}

bool JsonScene::containsTerrain() const{
	for(const auto& cur: objects){
		if (cur.hasTerrain())
			return true;
	}
	return false;
}
