#pragma once
#include "JsonObjects.h"

class JsonSkeletonBone{
public:
	FString name;
	int parent = -1;
	FMatrix pose;
	FMatrix invPose;

	void load(JsonObjPtr data);
	JsonSkeletonBone(JsonObjPtr data){
		load(data);
	}
	JsonSkeletonBone() = default;
};