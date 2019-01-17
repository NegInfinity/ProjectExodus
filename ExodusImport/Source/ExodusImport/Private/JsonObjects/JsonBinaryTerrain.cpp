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

void convertToUint8(DataPlane2D<uint8> &dstData, const FloatPlane2D &srcData){
	srcData.convertTo(dstData, 
		[](float arg) ->uint8{
			return FMath::Clamp(FMath::RoundToInt(arg * (float)0xFF), 0, 0xFF);
		}
	);
}

void convertFloat3DSplatToUintPlanes(TArray<DataPlane2D<uint8>> &outResult, const FloatPlane3D& src, int desiredW, int desiredH, const TCHAR* mapType = 0){
	if (!mapType)
		mapType = TEXT("");

	outResult.Empty();
	UE_LOG(JsonLogTerrain, Log, TEXT("Processing %s maps. %d detail maps present"), mapType, src.getNumLayers());
	for(int layerIndex = 0; layerIndex < src.getNumLayers(); layerIndex++){
		UE_LOG(JsonLogTerrain, Log, TEXT("Processing %s map %d out of %d."), mapType, layerIndex, src.getNumLayers());
		auto srcFloats = src.getLayerData(layerIndex);
		srcFloats.transpose();
		FloatPlane2D dstFloats(desiredW, desiredH);
		JsonTerrainTools::scaleSplatMapToHeightMap(dstFloats, srcFloats, true);

		auto& byteMap = outResult.AddDefaulted_GetRef();
		convertToUint8(byteMap, dstFloats);
		/*
		dstFloats.convertTo(byteMap, 
			[](float arg) ->uint8{
				return FMath::Clamp(FMath::RoundToInt(arg * (float)0xFF), 0, 0xFF);
			}
		);
		*/
	}
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
			float zeroLevel = (float)0x7FFF;
			float oneLevel = (float)0xFFFF;
			float diff = oneLevel - zeroLevel;
			return FMath::Clamp(FMath::RoundToInt(arg * diff + zeroLevel), 0, 0xFFFF);
			//return FMath::Clamp(FMath::RoundToInt(arg * (float)0xFFFF), 0, 0xFFFF);
		}
	);
	UE_LOG(JsonLogTerrain, Log, TEXT("Conversion finished. %d x %d"), heightMap.getWidth(), heightMap.getHeight());

	convertFloat3DSplatToUintPlanes(alphaMaps, src.alphaMaps, idealHMapW, idealHMapH, TEXT("alpha"));

	auto& dstDetails = detailMaps;
	const auto& srcDetails = src.detailMaps;
	UE_LOG(JsonLogTerrain, Log, TEXT("Processing %d detail maps"), srcDetails.getNumLayers());
	for(int detailIndex = 0 ; detailIndex < srcDetails.getNumLayers(); detailIndex++){
		UE_LOG(JsonLogTerrain, Log, TEXT("Processing detail map %d out of %d."), detailIndex, srcDetails.getNumLayers());

		auto srcLayer = srcDetails.getLayerData(detailIndex);
		srcLayer.transpose();

		FloatPlane2D srcFloats;
		srcLayer.convertTo(srcFloats, [](int32 arg)->float{
			return (float)FMath::Clamp(arg, 0, 16)/16.0f; //why? Is this an oversight?
		});

		FloatPlane2D dstFloats(idealHMapW, idealHMapH);//desiredW, desiredH);
		JsonTerrainTools::scaleSplatMapToHeightMap(dstFloats, srcFloats, true);

		auto& byteMap = dstDetails.AddDefaulted_GetRef();
		convertToUint8(byteMap, dstFloats);
		/*
		dstFloats.convertTo(byteMap, 
			[](float arg) ->uint8{
				return FMath::Clamp(FMath::RoundToInt(arg * (float)0xFF), 0, 0xFF);
			}
		);
		*/
		//srcLayer.

		/*
		FloatPlane2D srcFloats(srcLaye
		FloatPlane2D dstFloats(desiredW, desiredH);
		JsonTerrainTools::scaleSplatMapToHeightMap(dstFloats, srcFloats, true);

		auto& byteMap = outResult.AddDefaulted_GetRef();
		dstFloats.convertTo(byteMap, 
			[](float arg) ->uint8{
				return FMath::Clamp(FMath::RoundToInt(arg * (float)0xFF), 0, 0xFF);
			}
		);
		*/
	}
}
