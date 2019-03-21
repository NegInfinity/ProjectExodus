#pragma once
#include "JsonTypes.h"

class JsonTreePrototype{
public:
	JsonId prefabId = -1;
	JsonId prefabObjectId = -1;
	//JsonId meshId = -1;
	ResId meshId;// = ResId::invalid();
	IntArray materials;

	JsonTreePrototype() = default;
	JsonTreePrototype(JsonObjPtr data);
	void load(JsonObjPtr data);
};