#pragma once
#include "JsonTypes.h"

class JsonConfig{
public:
	void load(JsonObjPtr data);
	JsonConfig() = default;
	JsonConfig(JsonObjPtr data){
		load(data);
	}
};
