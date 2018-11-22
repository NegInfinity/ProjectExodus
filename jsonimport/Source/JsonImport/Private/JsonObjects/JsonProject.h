#pragma once
#include "JsonObjects.h"
#include "JsonConfig.h"
#include "JsonExternResourceList.h"

class JsonProject{
public:
	JsonConfig config;
	JsonExternResourceList externResources;
	
	void load(JsonObjPtr data);
	JsonProject() = default;
	JsonProject(JsonObjPtr data){
		load(data);
	}
};
