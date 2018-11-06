#pragma once

#include "JsonTypes.h"

class JsonImporter;
class ALandscape;

class TerrainBuildData{
public:
	FString terrainDataPath;
	JsonImporter *importer = nullptr;
	const JsonGameObject *gameObj = nullptr;
	const JsonTerrainData *terrData = nullptr;
	ALandscape *landscape = nullptr;
};