#pragma once
#include "JsonTypes.h"

class JsonBounds{
public:
	FVector center;
	FVector size;
	void load(JsonObjPtr jsonData);
	JsonBounds() = default;
	JsonBounds(JsonObjPtr jsonData);
};

