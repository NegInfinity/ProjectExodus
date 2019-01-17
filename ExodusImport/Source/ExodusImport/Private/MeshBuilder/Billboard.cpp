#include "JsonImportPrivatePCH.h"
#include "MeshBuilder.h"
#include "UnrealUtilities.h"

using namespace UnrealUtilities;

void MeshBuilder::generateBillboardMesh(UStaticMesh *staticMesh, UMaterialInterface *billboardMaterial){
	check(staticMesh);
	check(billboardMaterial);

	auto builderFunc = [&](FRawMesh& rawMesh, int lod) -> void{
		float defSize = 50.0f;
		TArray<FVector> verts = {
			FVector(0.0f, -defSize, defSize * 2.0f),
			FVector(0.0f, defSize, defSize * 2.0f),
			FVector(0.0f, defSize, 0.0f),
			FVector(0.0f, -defSize, 0.0f)
		};
		TArray<FVector2D> uvs = {
			FVector2D(0.0f, 0.0f),
			FVector2D(1.0f, 0.0f),
			FVector2D(1.0f, 1.0f),
			FVector2D(0.0f, 1.0f)
		};
		FVector n(-1.0f, 0.0f, 0.0f);
		TArray<FVector> normals = {
			n, n, n, n
		};
		//IntArray indices = {0, 2, 1, 0, 3, 2};
		IntArray indices = {0, 2, 1, 0, 3, 2, 0, 1, 2, 0, 2, 3};

		rawMesh.VertexPositions.SetNum(0);
		rawMesh.WedgeColors.SetNum(0);
		rawMesh.WedgeIndices.SetNum(0);
		for(int i = 0; i < MAX_MESH_TEXTURE_COORDS; i++)
			rawMesh.WedgeTexCoords[i].SetNum(0);
		rawMesh.WedgeColors.SetNum(0);
		rawMesh.WedgeTangentZ.SetNum(0);

		auto addIdx = [&](int32 idx) -> void{
			rawMesh.WedgeIndices.Add(idx);
			rawMesh.WedgeTangentZ.Add(normals[idx]);
			rawMesh.WedgeTexCoords[0].Add(uvs[idx]);
		};

		for(auto cur: verts){
			rawMesh.VertexPositions.Add(cur);
		}

		for(auto idx: indices){
			addIdx(idx);
		}

		rawMesh.FaceMaterialIndices.Add(0);
		rawMesh.FaceMaterialIndices.Add(0);

		rawMesh.FaceSmoothingMasks.Add(0);
		rawMesh.FaceSmoothingMasks.Add(0);

		rawMesh.FaceMaterialIndices.Add(0);
		rawMesh.FaceMaterialIndices.Add(0);

		rawMesh.FaceSmoothingMasks.Add(0);
		rawMesh.FaceSmoothingMasks.Add(0);
	};

	generateStaticMesh(staticMesh, builderFunc, nullptr, 
		[&](UStaticMesh* mesh, FStaticMeshSourceModel &model){
			mesh->StaticMaterials.Empty();
			mesh->StaticMaterials.Add(billboardMaterial);

			model.BuildSettings.bRecomputeNormals = false;
			model.BuildSettings.bRecomputeTangents = true;
		}
	);
}

