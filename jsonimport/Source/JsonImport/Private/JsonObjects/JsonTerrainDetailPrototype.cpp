#include "JsonImportPrivatePCH.h"
#include "JsonTerrainDetailPrototype.h"
#include "macros.h"

//#define JSON_ENABLE_VALUE_LOGGING

using namespace JsonObjects;

JsonTerrainDetailPrototype::JsonTerrainDetailPrototype(JsonObjPtr data){
	load(data);
}

void JsonTerrainDetailPrototype::load(JsonObjPtr data){
	JSON_GET_VAR(data, textureId);
	JSON_GET_VAR(data, detailPrefabId);
	JSON_GET_VAR(data, detailPrefabObjectId);

	JSON_GET_VAR(data, detailMeshId);
	JSON_GET_VAR(data, detailMeshMaterials);

	JSON_GET_VAR(data, bendFactor);
	JSON_GET_PARAM(data, dryColor, getColor);//Gamma?
	JSON_GET_PARAM(data, healthyColor, getColor);

	JSON_GET_VAR(data, minWidth);
	JSON_GET_VAR(data, minHeight);
	JSON_GET_VAR(data, maxHeight);
	JSON_GET_VAR(data, maxWidth);
	JSON_GET_VAR(data, noiseSpread);
	JSON_GET_VAR(data, renderMode);
	JSON_GET_VAR(data, usePrototypeMesh);

	billboardFlag = (renderMode == TEXT("GrassBillboard"));
	vertexLitFlag = (renderMode == TEXT("VertexLit"));
	grassFlag = (renderMode == TEXT("Grass"));
}
