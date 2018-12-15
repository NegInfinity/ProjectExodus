#include "JsonImportPrivatePCH.h"
#include "JsonGameObject.h"
#include "macros.h"

JsonGameObject::JsonGameObject(JsonObjPtr jsonData){
	load(jsonData);
}

void JsonGameObject::load(JsonObjPtr jsonData){
	JSON_GET_PARAM(jsonData, name, getString);
	JSON_GET_PARAM(jsonData, id, getInt);

	JSON_GET_PARAM(jsonData, scenePath, getString);

	JSON_GET_PARAM(jsonData, instanceId, getInt);
	JSON_GET_PARAM(jsonData, localPosition, getVector);
	JSON_GET_PARAM(jsonData, localRotation, getQuat);
	JSON_GET_PARAM(jsonData, localScale, getVector);
	JSON_GET_PARAM(jsonData, worldMatrix, getMatrix);
	JSON_GET_PARAM(jsonData, localMatrix, getMatrix);
	JSON_GET_PARAM2(jsonData, parentId, parent, getInt);

	JSON_GET_PARAM2(jsonData, parentName, parentName, getString);

	JSON_GET_PARAM2(jsonData, meshId, mesh, getInt);

	JSON_GET_PARAM(jsonData, activeSelf, getBool);
	JSON_GET_PARAM(jsonData, activeInHierarchy, getBool);

	JSON_GET_PARAM(jsonData, isStatic, getBool);
	JSON_GET_PARAM(jsonData, lightMapStatic, getBool);
	JSON_GET_PARAM(jsonData, navigationStatic, getBool);
	JSON_GET_PARAM(jsonData, occluderStatic, getBool);
	JSON_GET_PARAM(jsonData, occludeeStatic, getBool);

	JSON_GET_PARAM(jsonData, nameClash, getBool);
	JSON_GET_PARAM(jsonData, uniqueName, getString);

	JSON_GET_PARAM(jsonData, prefabRootId, getInt);
	JSON_GET_PARAM(jsonData, prefabObjectId, getInt);
	JSON_GET_PARAM(jsonData, prefabInstance, getBool);
	JSON_GET_PARAM(jsonData, prefabModelInstance, getBool);
	JSON_GET_PARAM(jsonData, prefabType, getString);

	renderers.Empty();
	skinRenderers.Empty();
	lights.Empty();
	probes.Empty();

	getJsonObjArray(jsonData, lights, "light", true);
	getJsonObjArray(jsonData, renderers, "renderer", true);
	getJsonObjArray(jsonData, probes, "reflectionProbes", true);
	getJsonObjArray(jsonData, terrains, "terrains", true);
	getJsonObjArray(jsonData, skinRenderers, "skinRenderers", true);

	if (nameClash && (uniqueName.Len() > 0)){
		UE_LOG(JsonLog, Warning, TEXT("Name clash detected on object %d: %s. Renaming to %s"), 
			id, *name, *uniqueName);		
		ueName = uniqueName;
	}
	else
		ueName = name;

	ueWorldMatrix = unityWorldToUe(worldMatrix);
}

IntArray JsonGameObject::getFirstMaterials() const{
	for(int i = 0; i < renderers.Num(); i++){
		const auto &curRend = renderers[i];
		if (curRend.hasMaterials())
			return curRend.materials;
	}
	UE_LOG(JsonLog, Warning, TEXT("Materials not found on renderer, returning blank array"));
	return IntArray();
}

