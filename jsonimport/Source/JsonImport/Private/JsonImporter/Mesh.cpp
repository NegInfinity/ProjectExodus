#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"

#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Classes/Components/PointLightComponent.h"
#include "Engine/Classes/Components/SpotLightComponent.h"
#include "Engine/Classes/Components/DirectionalLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "LevelEditorViewport.h"
#include "Factories/TextureFactory.h"
#include "Factories/MaterialFactoryNew.h"

#include "Materials/Material.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
	
#include "RawMesh.h"

#include "JsonObjects/JsonMesh.h"
#include "UnrealUtilities.h"
#include "JsonObjects/converters.h"

#include "DesktopPlatformModule.h"

using namespace UnrealUtilities;
using namespace JsonObjects;

static FVector2D getIdxVector2(const TArray<float>& floats, int32 idx){
	if (floats.Num() <= (idx*2 + 1))
		return FVector2D();
	return FVector2D(floats[idx*2], floats[idx*2+1]);
};

static FVector getIdxVector3(const TArray<float>& floats, int32 idx){
	if (floats.Num() <= (idx*3 + 2))
		return FVector();
	return FVector(floats[idx*3], floats[idx*3+1], floats[idx*3+2]);
};

void JsonImporter::importMesh(JsonObjPtr obj, int32 meshId){
	UE_LOG(JsonLog, Log, TEXT("Importing mesh %d"), meshId);

	JsonMesh jsonMesh(obj);

	UE_LOG(JsonLog, Log, TEXT("Mesh data: Verts: %d; submeshes: %d; materials: %d; colors %d; normals: %d"), 
		jsonMesh.verts.Num(), jsonMesh.subMeshes.Num(), jsonMesh.colors.Num(), jsonMesh.normals.Num());
	UE_LOG(JsonLog, Log, TEXT("Mesh data: uv0: %d; uv1: %d; uv2: %d; uv3: %d; uv4: %d; uv5: %d; uv6: %d; uv7: %d;"),
		jsonMesh.uv0.Num(), jsonMesh.uv1.Num(), jsonMesh.uv2.Num(), jsonMesh.uv3.Num(), 
		jsonMesh.uv4.Num(), jsonMesh.uv5.Num(), jsonMesh.uv6.Num(), jsonMesh.uv7.Num());
	
	if (jsonMesh.verts.Num() <= 0){
		UE_LOG(JsonLog, Warning, TEXT("No verts, cannot create mesh!"));
		return;
	}

	FString sanitizedMeshName;
	FString sanitizedPackageName;

	FString meshName = jsonMesh.name;
	auto pathBaseName = FPaths::GetBaseFilename(jsonMesh.path);
	if (!pathBaseName.IsEmpty()){
		//Well, I've managed to create a level with two meshes named "cube". So...
		meshName = FString::Printf(TEXT("%s_%s_%d"), *pathBaseName, *meshName, meshId);
	}
	else{
		meshName = FString::Printf(TEXT("%s_%d"), *meshName, meshId);
	}

	UStaticMesh *existingMesh = nullptr;
	UPackage *meshPackage = createPackage(
		meshName, jsonMesh.path, assetRootPath, FString("Mesh"), 
		&sanitizedPackageName, &sanitizedMeshName, &existingMesh);
	if (existingMesh){
		meshIdMap.Add(jsonMesh.id, existingMesh->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing mesh: %s (package %s)"), *sanitizedMeshName, *sanitizedPackageName);
		return;
	}

	UStaticMesh* mesh = NewObject<UStaticMesh>(meshPackage, FName(*sanitizedMeshName), RF_Standalone|RF_Public);

	if (!mesh){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't create mesh"));
		return;
	}

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

	mesh->StaticMaterials.Empty();
	for(auto matId: jsonMesh.materials){
		UMaterialInterface *material = loadMaterialInterface(matId);
		mesh->StaticMaterials.Add(material);
	}

	srcModel.RawMeshBulkData->SaveRawMesh(newRawMesh);

	srcModel.BuildSettings.bRecomputeNormals = false;//!hasNormals;//hasNormals
	srcModel.BuildSettings.bRecomputeTangents = true;

	TArray<FText> buildErrors;
	mesh->Build(false, &buildErrors);
	for(FText& err: buildErrors){
		UE_LOG(JsonLog, Error, TEXT("MeshBuildError: %s"), *(err.ToString()));
	}

	if (mesh){
		auto meshPath = mesh->GetPathName();
		meshIdMap.Add(jsonMesh.id, meshPath);
		FAssetRegistryModule::AssetCreated(mesh);
		meshPackage->SetDirtyFlag(true);
	}
}
