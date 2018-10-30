#pragma once

#include "JsonTypes.h"
#include "FloatMap2D.h"
#include "FloatMap3D.h"

class JsonBinaryTerrain{
public:
	FloatMap2D heightMap;
	FloatMap3D alphaMaps;
	FloatMap3D detailMaps;

	void clear();
	bool load(const FString &filename);
	JsonBinaryTerrain() = default;
};
