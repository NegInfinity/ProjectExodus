#include "JsonImportPrivatePCH.h"
#include "JsonBinaryTerrain.h"

void JsonBinaryTerrain::clear(){
	heightMap.clear();
	alphaMaps.clear();
	detailMaps.clear();
}

template<typename T> static void readPtr(T& out, uint8*&ptr){
	out = *((T*)ptr);
	ptr += sizeof(T);
}

template<typename T> void readPtrElements(T* dst, int32 numElements, uint8*&ptr){
	T* src = (T*)ptr;
	for(int i = 0; i < numElements; i++)
		dst[i] = src[i];

	ptr += sizeof(T) * numElements;//overflow?
}

bool JsonBinaryTerrain::load(const FString &filename){
	TArray<uint8> fileBuffer;
	if (!FFileHelper::LoadFileToArray(fileBuffer, *filename)){
		UE_LOG(JsonLog, Error, TEXT("Could not load binary terrain data from \"%s\""), *filename);
		return false;
	}

	auto fileSize = fileBuffer.Num();//Aaand nope.  - 2; ///As per documentation, there are two padding bytes at the end.... for some reason?

	int32 hMapW = 0, hMapH = 0, alphaMapW = 0, alphaMapH = 0, 
		numAlphaMaps = 0, detailMapW = 0, detailMapH = 0, numDetailMaps = 0;

	const auto headerSize = sizeof(hMapW) * 8;

	if (fileSize < headerSize){
		UE_LOG(JsonLog, Error, TEXT("File \"%s\" is too small to store a header. Min header size is %d"), 
			*filename, (int32)headerSize);
		return false;
	}

	uint8* basePtr = fileBuffer.GetData();
	auto curPtr = basePtr;

	readPtr(hMapW, curPtr);
	readPtr(hMapH, curPtr);
	readPtr(alphaMapW, curPtr);
	readPtr(alphaMapH, curPtr);
	readPtr(numAlphaMaps, curPtr);
	readPtr(detailMapW, curPtr);
	readPtr(detailMapH, curPtr);
	readPtr(numDetailMaps, curPtr);

	const auto dataSize = sizeof(float) * (
		hMapW * hMapH  + alphaMapW * alphaMapH * numAlphaMaps + detailMapW * detailMapH * numDetailMaps
	);

	const auto totalSize = headerSize + dataSize;

	if (fileSize <  totalSize){
		UE_LOG(JsonLog, Error, TEXT("File \"%s\" is too small to map data. Expected file size %d"), 
			*filename, (int32)totalSize);
		return false;
	}

	heightMap.resize(hMapW, hMapH);
	alphaMaps.resize(alphaMapW, alphaMapH, numAlphaMaps);
	detailMaps.resize(detailMapW, detailMapH, numDetailMaps);

	readPtrElements(heightMap.getData(), heightMap.getNumElements(), curPtr);
	readPtrElements(alphaMaps.getData(), alphaMaps.getNumElements(), curPtr);
	readPtrElements(detailMaps.getData(), detailMaps.getNumElements(), curPtr);

	return true;
}
