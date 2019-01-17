#pragma once
#include "JsonObjects.h"

class JsonSkeletonBone{
public:
	FString name;
	int id = -1;
	int parentId = -1;
	FMatrix world = FMatrix::Identity;
	FMatrix local = FMatrix::Identity;
	FMatrix rootRelative = FMatrix::Identity;

	void load(JsonObjPtr data);
	JsonSkeletonBone(JsonObjPtr data){
		load(data);
	}
	JsonSkeletonBone() = default;
};