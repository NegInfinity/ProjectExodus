#include "JsonImportPrivatePCH.h"
#include "FloatMap3D.h"

void FloatMap3D::resize(int32 width_, int32 height_, int32 numLayers_){
	width = width_;
	height = height_;
	layers = numLayers_;
	data.SetNum(width * height * layers);

	numLayerFloats = width * height;
	numTotalFloats = numLayerFloats * layers;
}

void FloatMap3D::clear(){
	resize(0, 0, 0);
}

float* FloatMap3D::getData(){
	return data.GetData();
}

const float* FloatMap3D::getData() const{
	return data.GetData();
}

float* FloatMap3D::getLayer(int layer){
	return &data[getLayerFloatSize() * layer];
}

const float* FloatMap3D::getLayer(int layer) const{
	return &data[getLayerFloatSize() * layer];
}

float* FloatMap3D::getRow(int layer, int y){
	return &data[getLayerFloatSize() * layer + getRowFloatSize() * y];
}

const float* FloatMap3D::getRow(int layer, int y) const{
	return &data[getLayerFloatSize() * layer + getRowFloatSize() * y];
}

float FloatMap3D::getValue(int layer, int x, int y) const{
	return data[getLayerFloatSize() * layer + getRowFloatSize() * y + x];
}

FloatMap3D::FloatMap3D(int width_, int height_, int numLayers_){
	resize(width_, height_, numLayers_);
}
