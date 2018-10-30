#pragma once

#include "JsonTypes.h"

class FloatMap3D{
protected:
	int32 width = 0;
	int32 height = 0;
	int32 layers = 0;
	FloatArray data;

	int32 numLayerFloats = 0;
	int32 numTotalFloats = 0;
public:
	int getWidth() const{return width;}
	int getHeight() const{return height;}
	int getNumLayers() const{return layers;}
	int getRowFloatSize() const{return width;}
	int getLayerFloatSize() const{return numLayerFloats;}
	int getFloatSize(){return numTotalFloats;}

	void resize(int32 width_, int32 height_, int32 numLayers_);
	void clear();
	float* getData();
	const float* getData() const;
	float* getLayer(int layer);
	const float* getLayer(int layer) const;

	float* getRow(int layer, int y);
	const float* getRow(int layer, int y) const;
	float getValue(int layer, int x, int y) const;
	FloatMap3D() = default;
	FloatMap3D(int width, int height, int numLayers);
};

