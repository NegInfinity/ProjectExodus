#pragma once
#include "JsonTypes.h"

class JsonSkinRenderer{
public:
	FString quality;
	bool skinnedMotionVectors;
	bool updateWhenOffscreen;

	StringArray boneNames;
	IntArray boneIds;
	MatrixArray boneTransforms;

	//int meshId = -1;
	ResId meshId;
	IntArray materials;

	void load(JsonObjPtr data);

	JsonSkinRenderer() = default;
	JsonSkinRenderer(JsonObjPtr data){
		load(data);
	}
};
