#include "JsonImportPrivatePCH.h"
#include "JsonMesh.h"
#include "macros.h"

using namespace JsonObjects;

void JsonSubMesh::load(JsonObjPtr data){
	JSON_GET_VAR(data, triangles);
}

void JsonMesh::load(JsonObjPtr data){
	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, name);
	JSON_GET_VAR(data, path);
	JSON_GET_VAR(data, materials);
	JSON_GET_VAR(data, readable);
	JSON_GET_VAR(data, vertexCount);
	//TArray<FLinearColor> colors;
	JSON_GET_VAR(data, colors);
	
	JSON_GET_VAR(data, verts);
	JSON_GET_VAR(data, normals);
	JSON_GET_VAR(data, uv0);
	JSON_GET_VAR(data, uv1);
	JSON_GET_VAR(data, uv2);
	JSON_GET_VAR(data, uv3);

	JSON_GET_VAR(data, subMeshCount);
	getJsonObjArray(data, subMeshes, "subMeshes");
}
