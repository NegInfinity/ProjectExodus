#include "JsonImportPrivatePCH.h"
#include "JsonSkeleton.h"
#include "macros.h"

using namespace JsonObjects;

void JsonSkeleton::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, name);
	//JSON_GET_VAR(data, defaultBoneNames);

	getJsonObjArray(data, bones, "bones");
}

int JsonSkeleton::findBoneIndex(const FString &boneName) const{
	for(int i = 0; i < bones.Num(); i++){
		if (bones[i].name == boneName)
			return i;
	}
	return -1;
}
