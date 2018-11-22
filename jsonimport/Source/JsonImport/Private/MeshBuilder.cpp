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

void MeshBuilder::setupMesh(UStaticMesh *mesh, const JsonMesh &jsonMesh, std::function<void(TArray<FStaticMaterial> &meshMaterial)> materialSetup){
	check(mesh);
	UE_LOG(JsonLog, Log, TEXT("Static mesh num lods: %d"), mesh->SourceModels.Num());

	if (mesh->SourceModels.Num() < 1){
		UE_LOG(JsonLog, Warning, TEXT("Adding static mesh lod!"));
		new(mesh->SourceModels) FStaticMeshSourceModel();//???
	}

	int32 lod = 0;

	FStaticMeshSourceModel &srcModel = mesh->SourceModels[lod];

	mesh->LightingGuid = FGuid::NewGuid();
	mesh->LightMapResolution = 64;
	mesh->LightMapCoordinateIndex = 1;

	FRawMesh newRawMesh;
	srcModel.RawMeshBulkData->LoadRawMesh(newRawMesh);
	newRawMesh.VertexPositions.SetNum(0);

	{
		UE_LOG(JsonLog, Log, TEXT("Generating mesh"));
		UE_LOG(JsonLog, Log, TEXT("Num vert floats: %d"), jsonMesh.verts.Num());//vertFloats.Num());
		for(int i = 0; (i + 2) < jsonMesh.verts.Num(); i += 3){
			FVector unityPos(jsonMesh.verts[i], jsonMesh.verts[i+1], jsonMesh.verts[i+2]);
			newRawMesh.VertexPositions.Add(unityPosToUe(unityPos));
		}
		UE_LOG(JsonLog, Log, TEXT("Num verts: %d"), newRawMesh.VertexPositions.Num());

		//const auto &normalFloats = jsonMesh.normals;
		UE_LOG(JsonLog, Log, TEXT("Num normal floats: %d"), jsonMesh.verts.Num());
		bool hasNormals = jsonMesh.normals.Num() != 0;
		UE_LOG(JsonLog, Log, TEXT("has normals: %d"), (int)hasNormals);
		bool hasColors = jsonMesh.colors.Num() > 0;

		const int32 maxUvs = 8;
		const TArray<float>* uvFloats[maxUvs] = {
			&jsonMesh.uv0, &jsonMesh.uv1, &jsonMesh.uv2, &jsonMesh.uv3, 
			&jsonMesh.uv4, &jsonMesh.uv5, &jsonMesh.uv6, &jsonMesh.uv7
		};

		bool hasUvs[maxUvs];
		for(int32 i = 0; i < maxUvs; i++){
			hasUvs[i] = uvFloats[i]->Num() != 0;
			UE_LOG(JsonLog, Log, TEXT("Uv floats[%d]: %d, hasUvs: %d"), i, uvFloats[i]->Num(), (int)hasUvs[i]);
		}

		//submesh generation
		newRawMesh.WedgeIndices.SetNum(0);

		for(int i = 0; i < MAX_MESH_TEXTURE_COORDS; i++)
			newRawMesh.WedgeTexCoords[i].SetNum(0);

		newRawMesh.WedgeColors.SetNum(0);
		newRawMesh.WedgeTangentZ.SetNum(0);

		UE_LOG(JsonLog, Log, TEXT("Sub meshes: %d"), jsonMesh.subMeshes.Num());
		if (jsonMesh.subMeshes.Num() > 0){
			UE_LOG(JsonLog, Log, TEXT("Processing submeshes"));
			int32 nextMatIndex = 0;

			for(int subMeshIndex = 0; subMeshIndex < jsonMesh.subMeshes.Num(); subMeshIndex++){
				const auto& curSubMesh = jsonMesh.subMeshes[subMeshIndex];

				const auto& trigs = curSubMesh.triangles;
				UE_LOG(JsonLog, Log, TEXT("Num triangle verts %d"), trigs.Num());

				auto processTriangleIndex = [&](int32 trigVertIdx){
					auto origIndex = trigs[trigVertIdx];
					newRawMesh.WedgeIndices.Add(origIndex);

					if (hasNormals){
						newRawMesh.WedgeTangentZ.Add(
							unityToUe(
								getIdxVector3(jsonMesh.normals, origIndex)
							)
						);
					}

					for(int32 uvIndex = 0; uvIndex < maxUvs; uvIndex++){
						if (!hasUvs[uvIndex])
							continue;

						auto tmpUv = getIdxVector2(*uvFloats[uvIndex], origIndex);
						tmpUv.Y = 1.0f - tmpUv.Y;
						newRawMesh.WedgeTexCoords[uvIndex].Add(tmpUv);
					}

					if (hasColors){
						FColor col32(
							jsonMesh.colors[trigVertIdx * 4], 
							jsonMesh.colors[trigVertIdx * 4 + 1], 
							jsonMesh.colors[trigVertIdx * 4 + 2], 
							jsonMesh.colors[trigVertIdx * 4 + 3]
						);

						//srgb conversion, though?
						newRawMesh.WedgeColors.Add(
							col32
						);
					}

					if ((trigVertIdx % 3) == 0){
						newRawMesh.FaceMaterialIndices.Add(subMeshIndex);
						newRawMesh.FaceSmoothingMasks.Add(0);
					}
				};

				for(int32 trigVertIndex = 0; (trigVertIndex + 2) < trigs.Num(); trigVertIndex += 3){
					processTriangleIndex(trigVertIndex);
					processTriangleIndex(trigVertIndex + 2);
					processTriangleIndex(trigVertIndex + 1);
				}

				UE_LOG(JsonLog, Log, TEXT("New wedge indices %d"), newRawMesh.WedgeIndices.Num());
				UE_LOG(JsonLog, Log, TEXT("Face mat indices: %d"), newRawMesh.FaceMaterialIndices.Num());
				for(int32 i = 0; i < MAX_MESH_TEXTURE_COORDS; i++){
					UE_LOG(JsonLog, Log, TEXT("Uv[%d] size: %d"), i, newRawMesh.WedgeTexCoords[i].Num());
				}
			}
		}
		else{
			UE_LOG(JsonLog, Warning, TEXT("No Submeshes found!"));
		}
	}

	bool valid = newRawMesh.IsValid();
	bool fixable = newRawMesh.IsValidOrFixable();
	UE_LOG(JsonLog, Log, TEXT("Mesh is valid: %d, mesh is validOrFixable: %d"), (int)valid, (int)fixable);
	if (!valid){
		UE_LOG(JsonLog, Warning, TEXT("Mesh is not valid!"));
		if (!fixable){
			UE_LOG(JsonLog, Warning, TEXT("Mesh is not fixable!"));
		}
	}

	if (materialSetup){
		materialSetup(mesh->StaticMaterials);
	}
	/*
	mesh->StaticMaterials.Empty();
	for(auto matId: jsonMesh.materials){
		UMaterialInterface *material = loadMaterialInterface(matId);
		mesh->StaticMaterials.Add(material);
	}
	*/

	srcModel.RawMeshBulkData->SaveRawMesh(newRawMesh);

	srcModel.BuildSettings.bRecomputeNormals = false;//!hasNormals;//hasNormals
	srcModel.BuildSettings.bRecomputeTangents = true;

	TArray<FText> buildErrors;
	mesh->Build(false, &buildErrors);
	for(FText& err: buildErrors){
		UE_LOG(JsonLog, Error, TEXT("MeshBuildError: %s"), *(err.ToString()));
	}	
}
