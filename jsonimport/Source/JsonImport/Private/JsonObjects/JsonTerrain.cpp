#include "JsonImportPrivatePCH.h"
#include "JsonTerrain.h"
#include "macros.h"

JsonTerrain::JsonTerrain(JsonObjPtr data){
	load(data);
}

void JsonTerrain::load(JsonObjPtr data){
	JSON_GET_PARAM(data, castShadows, getBool);
	JSON_GET_PARAM(data, detailObjectDensity, getFloat);
	JSON_GET_PARAM(data, detailObjectDistance, getFloat);
	JSON_GET_PARAM(data, drawHeightmap, getBool);
	JSON_GET_PARAM(data, drawTreesAndFoliage, getBool);
		
	JSON_GET_PARAM(data, renderHeightmap, getBool);
	JSON_GET_PARAM(data, renderTrees, getBool);
	JSON_GET_PARAM(data, renderDetails, getBool);
		
	JSON_GET_PARAM(data, heightmapPixelError, getFloat);
		
	JSON_GET_PARAM(data, legacyShininess, getFloat);
	//Color legacySpecular;
	JSON_GET_PARAM(data, legacySpecular, getColor);
	JSON_GET_PARAM(data, lightmapIndex, getInt);
	JSON_GET_PARAM(data, lightmapScaleOffet, getVector4);

	JSON_GET_PARAM(data, materialTemplateIndex, getInt);
	JSON_GET_PARAM(data, materialType, getString);
		
	JSON_GET_PARAM(data, patchBoundsMultiplier, getVector);
	JSON_GET_PARAM(data, preserveTreePrototypeLayers,  getBool);
	JSON_GET_PARAM(data, realtimeLightmapIndex, getInt);
	JSON_GET_PARAM(data, realtimeLightmapScaleOffset, getVector4);
		
	JSON_GET_PARAM(data, terrainDataId, getInt);
		
	JSON_GET_PARAM(data, treeBillboardDistance, getFloat);
	JSON_GET_PARAM(data, treeCrossFadeLength, getFloat);
		
	JSON_GET_PARAM(data, treeDistance, getFloat);
	JSON_GET_PARAM(data, treeLodBiasMultiplier, getFloat);
		
	JSON_GET_PARAM(data, treeMaximumFullLODCount, getInt);
}

