#include "JsonImportPrivatePCH.h"
#include "JsonSkinRenderer.h"

#include "macros.h"

using namespace JsonObjects;

void JsonSkinRenderer::load(JsonObjPtr data){
	JSON_GET_VAR(data, quality);
	JSON_GET_VAR(data, skinnedMotionVectors);
	JSON_GET_VAR(data, updateWhenOffscreen);

	JSON_GET_PARAM(data, boneNames, getStringArray);
	JSON_GET_PARAM(data, boneIds, getIntArray);
	JSON_GET_PARAM(data, boneTransforms, getMatrixArray);

	JSON_GET_VAR(data, meshId);
	JSON_GET_PARAM(data, materials, getIntArray);
}

