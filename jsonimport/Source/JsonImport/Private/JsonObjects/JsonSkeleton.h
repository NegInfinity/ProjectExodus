#pragma once
#include "JsonObjects.h"
#include "JsonSkeletonBone.h"

class JsonSkeleton{
public:
	int id = -1;
	FString name;

	TArray<JsonSkeletonBone> bones;

	void load(JsonObjPtr data);
	JsonSkeleton() = default;
	JsonSkeleton(JsonObjPtr data){
		load(data);
	}
};