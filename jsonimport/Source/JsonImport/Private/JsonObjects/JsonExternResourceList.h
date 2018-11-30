#pragma once
#include "JsonTypes.h"

class JsonExternResourceList{
public:
	StringArray scenes;
	StringArray materials;
	StringArray skeletons;
	StringArray meshes;
	StringArray textures;
	StringArray prefabs;
	StringArray terrains;
	StringArray cubemaps;
	StringArray audioClips;
	StringArray resources;

	void load(JsonObjPtr data);
	JsonExternResourceList() = default;
	JsonExternResourceList(JsonObjPtr data){
		load(data);
	}
};