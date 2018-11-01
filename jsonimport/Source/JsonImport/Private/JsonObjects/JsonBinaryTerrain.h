#pragma once

#include "JsonTypes.h"
#include "DataPlane2D.h"
#include "DataPlane3D.h"
#include "terrainTools.h"

//using FloatPlane2D = DataPlane2D<float>;
using FloatPlane3D = DataPlane3D<float>;

class JsonBinaryTerrain{
public:
	FloatPlane2D heightMap;
	FloatPlane3D alphaMaps;
	FloatPlane3D detailMaps;

	void clear();
	bool load(const FString &filename);
	JsonBinaryTerrain() = default;
};

class JsonConvertedTerrain{
public:
	DataPlane2D<uint16> heightMap;
	DataPlane3D<uint8> alphaMaps;
	//detail maps???

	void clear();
	void assignFrom(const JsonBinaryTerrain& src);
	JsonConvertedTerrain() = default;
};