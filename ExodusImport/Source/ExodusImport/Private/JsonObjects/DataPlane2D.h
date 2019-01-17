#pragma once

#include "JsonTypes.h"
#include "DataPlaneUtility.h"

template<typename T> class DataPlane2D{
protected:
	int32 width = 0;
	int32 height = 0;
	using DataArray = TArray<T>;
	DataArray data;
	int32 numTotalEls = 0;
public:
	int32 getWidth() const{return width;}
	int32 getHeight() const{return height;}
	int getNumRowElements() const{return width;}
	int32 getNumElements() const{return numTotalEls;}

	bool isEmpty() const{
		return (width == 0) || (height == 0);
	}

	void transpose(){
		auto oldData = data;
		auto srcWidth = getWidth();
		auto srcHeight = getHeight();
		resize(srcHeight, srcWidth);
		DataPlaneUtility::transpose2dData(getData(), oldData.GetData(), srcWidth, srcHeight);
	}

	DataPlane2D<T> getTransposed() const{
		auto srcWidth = getWidth();
		auto srcHeight = getHeight();
		DataPlane2D<T> result(srcHeight, srcWidth);
		DataPlaneUtility::transpose2dData(result.getData(), getData(), srcWidth, srcHeight);
		return result;
	}

	void resize(int32 width_, int32 height_){
		width = width_;
		height = height_;
		data.SetNum(width * height);
		numTotalEls = width * height;
	}

	void clear(){
		resize(0, 0);
	}

	int32 getByteSize() const{
		return sizeof(T) * getNumElements();
	}

	T* getData(){
		return data.GetData();
	}

	const T* getData() const{
		return data.GetData();
	}

	T* getRow(int y){
		return &data[y * width];
	}

	const T* getRow(int y) const{
		return &data[y * getNumRowElements()];
	}

	T getValue(int x, int y) const{
		return data[x + y * getNumRowElements()];
	}

	const DataArray& getArray() const{
		return data;
	}

	DataArray getArrayCopy() const{
		return data;
	}

	void saveToRaw(const FString& filename) const{
		auto totalDataSize = sizeof(T) * numTotalEls;
		const uint8* dataPtr = (const uint8*)data.GetData();
		TArrayView<const uint8> view(dataPtr, totalDataSize);
		//TArrayView<const uint8> view(data.GetData(), data.Num());
		FFileHelper::SaveArrayToFile(view, *filename);
	}

	bool loadFromRaw(const FString& filename, int w_, int h_){
		resize(w_, h_);
		TArray<uint8> data;
		if (!FFileHelper::LoadFileToArray(data, *filename)){
			UE_LOG(JsonLog, Error, TEXT("Could not load file \"%s\""), *filename);
			return false;
		}
		auto byteSize = getByteSize();
		if (byteSize != data.Num()){
			UE_LOG(JsonLog, Error, TEXT("Invalid file size of \"%s\": %d received, %d (%d x %d (%d)) expected"),
				*filename, data.Num(), byteSize, getWidth(), getHeight(), sizeof(T));
			return false;
		}
		const T* srcPtr = (const T*)data.GetData();
		T* dstPtr = getData();
		for(auto i = 0; i < getNumElements(); i++){
			dstPtr[i] = srcPtr[i];
		}
		return true;
	}

	template<typename SrcT, class Converter> void assignFrom(const DataPlane2D<SrcT> &other, Converter converter){
		resize(other.getWidth(), other.getHeight());
		auto numElements = getNumElements();
		auto* dstPtr = getData();
		auto* srcPtr = other.getData();
		for(auto i = 0; i < numElements; i++){
			dstPtr[i] = converter(srcPtr[i]);
		}
	}

	template<typename DstT, class Converter> DataPlane2D<DstT> convertTo(Converter convert) const{
		DataPlane2D<DstT> result;
		result.assignFrom(*this, convert);
		return result;
	}

	template<typename DstT, class Converter> void convertTo(DataPlane2D<DstT> &out, Converter convert) const{
		out.assignFrom(*this, convert);
	}

	DataPlane2D() = default;
	DataPlane2D(int width, int height){
		resize(width, height);
	}
};
