#pragma once

#include "JsonTypes.h"
#include "DataPlane2D.h"

template<typename T> class DataPlane3D{
protected:
	int32 width = 0;
	int32 height = 0;
	int32 layers = 0;
	using DataArray = TArray<T>;
	DataArray data;

	int32 numLayerEls = 0;
	int32 numTotalEls = 0;
public:
	int getWidth() const{return width;}
	int getHeight() const{return height;}
	int getNumLayers() const{return layers;}
	int getNumRowElements() const{return width;}
	int getNumLayerElements() const{return numLayerEls;}
	int getNumElements(){return numTotalEls;}

	void resize(int32 width_, int32 height_, int32 numLayers_){
		width = width_;
		height = height_;
		layers = numLayers_;
		data.SetNum(width * height * layers);

		numLayerEls = width * height;
		numTotalEls = numLayerEls * layers;
	}

	void clear(){
		resize(0, 0, 0);
	}

	T* getData(){
		return data.GetData();
	}

	const T* getData() const{
		return data.GetData();
	}

	T* getLayer(int layer){
		return &data[getNumLayerElements() * layer];
	}

	const T* getLayer(int layer) const{
		return &data[getNumLayerElements() * layer];
	}

	T* getRow(int layer, int y){
		return &data[getNumLayerElements() * layer + getNumRowElements() * y];
	}

	const T* getRow(int layer, int y) const{
		return &data[getNumLayerElements() * layer + getNumRowElements() * y];
	}

	T getValue(int layer, int x, int y) const{
		return data[getNumLayerElements() * layer + getNumRowElements() * y + x];
	}

	void getLayerData(DataPlane2D<T> &result, int layer) const{
		result.resize(getWidth(), getHeight());
		auto* dst = result.getData();
		auto* src = getLayer(layer);
		for(auto i = 0; i < getNumLayerElements(); i++){
			dst[i] = src[i];
		}
	}

	DataPlane2D<T> getLayerData(int layer) const{
		DataPlane2D<T> result;
		getLayer(result, layer);
		return result;
	}

	template<typename Dst, typename Converter> void getLayerData(DataPlane2D<Dst> &result, int layer, Converter converter) const{
		result.resize(getWidth(), getHeight());
		auto* dst = result.getData();
		auto* src = getLayer(layer);
		for(auto i = 0; i < getNumLayerElements(); i++){
			dst[i] = converter(src[i]);
		}
	}

	template<typename Dst, typename Converter> DataPlane2D<Dst> getLayerData(int layer, Converter converter) const{
		DataPlane2D<Dst> result;
		getLayer(result, layer, converter);
		return result;
	}

	template<typename SrcT, class Converter> void assignFrom(const DataPlane3D<SrcT> &other, Converter converter){
		resize(other.getWidth(), other.getHeight());
		auto numElements = getNumElements();
		auto* dstPtr = getData();
		auto* srcPtr = other.getData();
		for(auto i = 0; i < numElements; i++){
			dstPtr[i] = converter(srcPtr[i]);
		}
	}

	template<typename DstT, class Converter> DataPlane3D<DstT> convertTo(Converter convert){
		DataPlane3D<DstT> result;
		result.assignFrom(*this, convert);
		return result;
	}

	template<typename DstT, class Converter> void convertTo(DataPlane3D<DstT> &out, Converter convert){
		out.assignFrom(*this, convert);
	}

	DataPlane3D(int width_, int height_, int numLayers_){
		resize(width_, height_, numLayers_);
	}
	DataPlane3D() = default;
};

