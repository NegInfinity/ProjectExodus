#pragma once

#include "JsonTypes.h"

class FloatMap2D{
protected:
	int32 width = 0;
	int32 height = 0;
	FloatArray data;
	int32 numTotalFloats = 0;
public:
	int32 getWidth(){return width;}
	int32 getHeight(){return height;}
	int getRowFloatSize() const{return width;}
	int32 getFloatSize() const{return numTotalFloats;}

	bool isEmpty() const;
	void clear();
	void resize(int32 width_, int32 height_);
	float* getData();
	const float* getData() const;

	//in floats
	float* getRow(int y);
	const float* getRow(int y) const;
	float getValue(int x, int y) const;
	FloatMap2D() = default;
	FloatMap2D(int width, int height);
};
