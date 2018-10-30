#include "JsonImportPrivatePCH.h"
#include "FloatMap2D.h"

void FloatMap2D::clear(){
	resize(0, 0);
}

float* FloatMap2D::getData(){
	return data.GetData();
}

const float* FloatMap2D::getData() const{
	return data.GetData();
}

float* FloatMap2D::getRow(int y){
	return &data[y * width];
}

const float* FloatMap2D::getRow(int y) const{
	return &data[y * getRowFloatSize()];
}

float FloatMap2D::getValue(int x, int y) const{
	return data[x + y * getRowFloatSize()];
}

FloatMap2D::FloatMap2D(int width, int height){
	resize(width, height);
}

void FloatMap2D::resize(int32 width_, int32 height_){
	width = width_;
	height = height_;
	data.SetNum(width * height);
	numTotalFloats = width * height;
}

bool FloatMap2D::isEmpty() const{
	return (width == 0) || (height == 0);
}
