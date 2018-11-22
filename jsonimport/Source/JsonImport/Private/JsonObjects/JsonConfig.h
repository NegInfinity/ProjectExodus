#pragma once
#include "JsonObjects.h"

class JsonConfig{
public:
	void load(JsonObjPtr data);
	JsonConfig() = default;
	JsonConfig(JsonObjPtr data){
		load(data);
	}
};
