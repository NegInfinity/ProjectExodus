#include "JsonImportPrivatePCH.h"
#include "JsonBinaryTerrain.h"
#include "terrainTools.h"

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

void JsonConvertedTerrain::clear(){
	heightMap.clear();
	alphaMaps.Empty();
}

bool isValidLandscapeSize(int size){
	return ((size - 1) % JsonTerrainConstants::quadsPerComponent) == 0;
}

int32 findCloseLandscapeSize(int srcVertSize){
	if (isValidLandscapeSize(srcVertSize))
		return srcVertSize;

	auto strips = srcVertSize - 1;
	auto comps = strips / JsonTerrainConstants::quadsPerComponent;
	if ((strips % JsonTerrainConstants::quadsPerComponent) != 0)
		comps ++;

	comps = FMath::Clamp(comps, 1, (int)JsonTerrainConstants::maxComponentsPerDimension);
	return comps * JsonTerrainConstants::quadsPerComponent + 1;
}

void JsonConvertedTerrain::assignFrom(const JsonBinaryTerrain& src){
	UE_LOG(JsonLogTerrain, Log, TEXT("Transposing height map"));
	auto floatHMap = src.heightMap.getTransposed();

	auto hMapW = floatHMap.getWidth();
	auto hMapH = floatHMap.getHeight();
	auto detMapH = src.detailMaps.getHeight();
	auto detMapW = src.detailMaps.getWidth();
	auto alphaMapW = src.alphaMaps.getWidth();
	auto alphaMapH = src.alphaMaps.getHeight();

	auto maxW = FMath::Max3(hMapW, detMapW, alphaMapW);
	auto maxH = FMath::Max3(hMapH, detMapH, alphaMapH);

	auto idealHMapW = findCloseLandscapeSize(maxW);//findCloseLandscapeSize(hMapW * 4);
	auto idealHMapH = findCloseLandscapeSize(maxH);//findCloseLandscapeSize(hMapH * 4);
	UE_LOG(JsonLogTerrain, Log, TEXT("Landscape width found: %d x %d"), idealHMapW, idealHMapH);

	if ((hMapW != idealHMapW) || (hMapH != idealHMapH)){
		UE_LOG(JsonLogTerrain, Log, TEXT("Resizing heightmap data"));
		//auto newHeight = conv
		FloatPlane2D newHMap(idealHMapW, idealHMapH);
		JsonTerrainTools::rescaleHeightMap(newHMap, floatHMap, true);
		floatHMap = newHMap;
	}
	else{
		UE_LOG(JsonLogTerrain, Log, TEXT("Sizes match (how?). Nothing to do."));
	}

	UE_LOG(JsonLogTerrain, Log, TEXT("Converting height map"));
	floatHMap.convertTo(
		heightMap, 
		[](float arg) ->uint16{
			return FMath::Clamp(FMath::RoundToInt(arg * (float)0xFFFF), 0, 0xFFFF);
		}
	);
	UE_LOG(JsonLogTerrain, Log, TEXT("Conversion finished. %d x %d"), heightMap.getWidth(), heightMap.getHeight());

	UE_LOG(JsonLogTerrain, Log, TEXT("Processing alpha maps. %d alpha maps present"), src.alphaMaps.getNumLayers());
	alphaMaps.Empty();
	for(int alphaIndex = 0; alphaIndex < src.alphaMaps.getNumLayers(); alphaIndex++){
		UE_LOG(JsonLogTerrain, Log, TEXT("Processing alpha maps %d out of %d."), alphaIndex, src.alphaMaps.getNumLayers());
		auto srcFloats = src.alphaMaps.getLayerData(alphaIndex);
		srcFloats.transpose();
		FloatPlane2D dstFloats(idealHMapW, idealHMapH);
		JsonTerrainTools::scaleSplatMapToHeightMap(dstFloats, srcFloats, true);

		auto& byteMap = alphaMaps.AddDefaulted_GetRef();
		dstFloats.convertTo(byteMap, 
			[](float arg) ->uint8{
				return FMath::Clamp(FMath::RoundToInt(arg * (float)0xFF), 0, 0xFF);
			}
		);

		/*
		auto filePath = FString::Printf(TEXT("%d.bin"), alphaIndex);
		auto debugPath = FPaths::Combine(TEXT("D:\\work\\EpicGames\\debug"), *filePath);

		auto bytes = byteMap.getArrayCopy();
		FFileHelper::SaveArrayToFile(bytes, *debugPath);
		*/
	}
}
