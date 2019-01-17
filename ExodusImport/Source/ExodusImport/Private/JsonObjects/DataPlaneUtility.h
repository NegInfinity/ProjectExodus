#pragma once

namespace DataPlaneUtility{
	template<typename T> void transpose2dData(T* transposed, const T* src, int32 srcWidth, int32 srcHeight){
		auto srcRowData = src;
		auto dstRowData = transposed;
		for(int32 y = 0; y < srcHeight; y++){
			auto srcPixData = srcRowData;
			auto dstPixData = dstRowData;
			for(int32 x = 0; x < srcWidth; x++){
				*dstPixData = *srcPixData;
				srcPixData ++;
				dstPixData += srcWidth;
			}
			srcRowData += srcWidth;
			dstRowData ++;
		}
	}

	//
	template<typename T> void transpose3dDataWidthHeight(T* transposed, const T* src, int32 srcWidth, int32 srcHeight, int32 srcDepth){
		auto srcRowData = src;
		auto dstRowData = transposed;
		auto layerSize=  srcWidth * srcHeight;
		for(int32 layer = 0; layer < srcDepth; layer++){
			transpose2dData(dstRowData, srcRowData, srcWidth, srcHeight);
			srcRowData += layerSize;
			dstRowData += layerSize;
		}
	}
}
