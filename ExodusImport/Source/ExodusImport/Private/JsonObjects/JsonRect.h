#pragma once

#include "JsonTypes.h"

class JsonRect{
public:
	FVector2D minPoint;
	FVector2D maxPoint;
	void load(JsonObjPtr jsonData);
	JsonRect() = default;
	JsonRect(JsonObjPtr jsonData){
		load(jsonData);
	}
};

