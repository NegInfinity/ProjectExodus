#pragma once
#include "JsonObjects.h"
#include "JsonSkeletonBone.h"

class JsonSkeleton{
public:
	int id = -1;
	FString name;
	//StringArray defaultBoneNames;

	TArray<JsonSkeletonBone> bones;
	int findBoneIndex(const FString &name) const;

	void load(JsonObjPtr data);
	JsonSkeleton() = default;
	JsonSkeleton(JsonObjPtr data){
		load(data);
	}
};