#pragma once
#include "JsonTypes.h"
#include "JsonObjects/JsonPrefabData.h"

class JsonImporter;

class PrefabBuilder{
protected:
	void sanityCheck(const JsonPrefabData &prefabData);
public:
	void importPrefab(const JsonPrefabData& prefabData, JsonImporter *importer);
};
