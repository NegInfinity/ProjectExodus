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

#include "DesktopPlatformModule.h"


void JsonImporter::importMesh(JsonObjPtr obj, int32 meshId){
	UE_LOG(JsonLog, Log, TEXT("Importing mesh %d"), meshId);
#define GETPARAM(name, op) auto name = op(obj, #name); logValue(#name, name);
	GETPARAM(id, getInt)
	GETPARAM(vertexCount, getInt)
	GETPARAM(path, getString)
	GETPARAM(name, getString)
	GETPARAM(subMeshCount, getInt)
#undef GETPARAM
	//auto verts = getObject(obj, "verts");
	//auto materials = getObject(obj, "materials");

	const JsonValPtrs *verts = 0, *materials = 0, *colors = 0, *normals = 0, *uv0 = 0, *uv1 = 0, *uv2 = 0, *uv3 = 0, *subMeshes = 0;

	loadArray(obj, verts, TEXT("verts"));
	if (verts){
		UE_LOG(JsonLog, Log, TEXT("Verts: %d"), verts->Num());
	}
	loadArray(obj, subMeshes, TEXT("submeshes"));
	if (subMeshes){
		UE_LOG(JsonLog, Log, TEXT("submeshes: %d"), subMeshes->Num());
	}
	loadArray(obj, materials, TEXT("materials"));
	if (materials){
		UE_LOG(JsonLog, Log, TEXT("Materials: %d"), materials->Num());
	}
	loadArray(obj, colors, TEXT("colors"));
	if (colors){
		UE_LOG(JsonLog, Log, TEXT("Colors: %d"), colors->Num());
	}
	loadArray(obj, normals, TEXT("normals"));
	if (normals){
		UE_LOG(JsonLog, Log, TEXT("Normals: %d"), normals->Num());
	}
	loadArray(obj, uv0, TEXT("uv0"));
	if (uv0){
		UE_LOG(JsonLog, Log, TEXT("uv0: %d"), uv0->Num());
	}
	loadArray(obj, uv1, TEXT("uv1"));
	if (uv1){
		UE_LOG(JsonLog, Log, TEXT("uv1: %d"), uv1->Num());
	}
	loadArray(obj, uv2, TEXT("uv2"));
	if (uv2){
		UE_LOG(JsonLog, Log, TEXT("uv2: %d"), uv2->Num());
	}
	loadArray(obj, uv3, TEXT("uv3"));
	if (uv3){
		UE_LOG(JsonLog, Log, TEXT("uv3: %d"), uv3->Num());
	}

	if (!verts){
		UE_LOG(JsonLog, Warning, TEXT("No verts, cannot create mesh!"));
		return;
	}

	FString sanitizedMeshName;
	FString sanitizedPackageName;

	UStaticMesh *existingMesh = nullptr;
	UPackage *meshPackage = createPackage(
		name, path, assetRootPath, FString("Mesh"), 
		&sanitizedPackageName, &sanitizedMeshName, &existingMesh);
	if (existingMesh){
		meshIdMap.Add(id, existingMesh->GetPathName());
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

	bool hasNormals = false;
	{
		UE_LOG(JsonLog, Log, TEXT("Generating mesh"));
		auto vertFloats = toFloatArray(verts);
		UE_LOG(JsonLog, Log, TEXT("Num vert floats: %d"), vertFloats.Num());
		float scale = 100.0f;
		for(int i = 0; (i + 2) < vertFloats.Num(); i += 3){
			FVector pos(vertFloats[i], vertFloats[i+1], vertFloats[i+2]);
			newRawMesh.VertexPositions.Add(unityToUe(pos * scale));
		}
		UE_LOG(JsonLog, Log, TEXT("Num verts: %d"), newRawMesh.VertexPositions.Num());

		int32 faceIndex = 0;

		auto normalFloats = toFloatArray(normals);
		UE_LOG(JsonLog, Log, TEXT("Num normal floats: %d"), vertFloats.Num());
		hasNormals = normalFloats.Num() != 0;
		UE_LOG(JsonLog, Log, TEXT("has normals: %d"), (int)hasNormals);
		const int32 maxUvs = 4;

		TArray<float> uvFloats[maxUvs] = {
			toFloatArray(uv0), toFloatArray(uv1), toFloatArray(uv2), toFloatArray(uv3)
		};
		bool hasUvs[maxUvs];
		for(int32 i = 0; i < maxUvs; i++){
			hasUvs[i] = uvFloats[i].Num() != 0;
			UE_LOG(JsonLog, Log, TEXT("Uv floats[%d]: %d, hasUvs: %d"), i, uvFloats[i].Num(), (int)hasUvs[i]);
		}

		//submesh generation
		newRawMesh.WedgeIndices.SetNum(0);

		for(int i = 0; i < MAX_MESH_TEXTURE_COORDS; i++)
			newRawMesh.WedgeTexCoords[i].SetNum(0);

		newRawMesh.WedgeColors.SetNum(0);
		newRawMesh.WedgeTangentZ.SetNum(0);

		UE_LOG(JsonLog, Log, TEXT("Sub meshes: %d"), (int)(subMeshes? subMeshes->Num(): 0));

		if (subMeshes){
			UE_LOG(JsonLog, Log, TEXT("Processing submeshes"));
			int32 nextMatIndex = 0;
			auto getIdxVector2 = [](TArray<float>& floats, int32 idx){
				if (floats.Num() <= (idx*2 + 1))
					return FVector2D();
				return FVector2D(floats[idx*2], floats[idx*2+1]);
			};

			auto getIdxVector3 = [](TArray<float>& floats, int32 idx){
				if (floats.Num() <= (idx*3 + 2))
					return FVector();
				return FVector(floats[idx*3], floats[idx*3+1], floats[idx*3+2]);
			};

			for(auto cur: *subMeshes){
				auto matIndex = nextMatIndex;
				nextMatIndex++;				
				UE_LOG(JsonLog, Log, TEXT("Processing submesh %d"), matIndex);
				auto trigJson = cur->AsObject();
				if (!trigJson.IsValid())
					continue;
				const JsonValPtrs* trigObj = 0;
				loadArray(trigJson, trigObj, "Triangles");
				if (!trigObj)
					continue;
				auto trigs = toIntArray(*trigObj);
				UE_LOG(JsonLog, Log, TEXT("Num triangle verts %d"), trigs.Num());

				auto processIndex = [&](int32 trigVertIdx){
					auto origIndex = trigs[trigVertIdx];
					newRawMesh.WedgeIndices.Add(origIndex);
					if (hasNormals)
						newRawMesh.WedgeTangentZ.Add(unityToUe(getIdxVector3(normalFloats, origIndex)));
					for(int32 uvIndex = 0; uvIndex < maxUvs; uvIndex++){
						if (!hasUvs[uvIndex])
							continue;
						auto tmpUv = getIdxVector2(uvFloats[uvIndex], origIndex);
						tmpUv.Y = 1.0f - tmpUv.Y;
						newRawMesh.WedgeTexCoords[uvIndex].Add(tmpUv);
					}
					if ((trigVertIdx % 3) == 0){
						newRawMesh.FaceMaterialIndices.Add(matIndex);
						newRawMesh.FaceSmoothingMasks.Add(0);
					}
				};

				for(int32 trigVertIndex = 0; (trigVertIndex + 2) < trigs.Num(); trigVertIndex += 3){
					processIndex(trigVertIndex);
					processIndex(trigVertIndex + 2);
					processIndex(trigVertIndex + 1);
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
	if (materials){
		auto matIds = toIntArray(*materials);
		for(auto matId: matIds){
			UMaterial *material = loadMaterial(matId);
			//mesh->Materials.Add(material);
			mesh->StaticMaterials.Add(material);
		}
	}

	srcModel.RawMeshBulkData->SaveRawMesh(newRawMesh);

	srcModel.BuildSettings.bRecomputeNormals = false;//!hasNormals;//hasNormals
	srcModel.BuildSettings.bRecomputeTangents = true;

	TArray<FText> buildErrors;
	mesh->Build(false, &buildErrors);
	for(FText& err: buildErrors){
		UE_LOG(JsonLog, Error, TEXT("MeshBuildError: %s"), *(err.ToString()));
	}
	//srcModel.BuildSettings

	if (mesh){
		meshIdMap.Add(id, mesh->GetPathName());
		FAssetRegistryModule::AssetCreated(mesh);
		meshPackage->SetDirtyFlag(true);
	}
}
