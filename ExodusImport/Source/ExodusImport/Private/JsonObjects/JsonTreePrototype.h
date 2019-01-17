#pragma once
#include "JsonTypes.h"

class JsonTreePrototype{
public:
	JsonId prefabId = -1;
	JsonId prefabObjectId = -1;
	JsonId meshId = -1;
	IntArray materials;

	JsonTreePrototype() = default;
	JsonTreePrototype(JsonObjPtr data);
	void load(JsonObjPtr data);
};