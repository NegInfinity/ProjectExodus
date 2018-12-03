#include "JsonImportPrivatePCH.h"
#include "MeshBuilder.h"
#include "UnrealUtilities.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshModel.h"
#include "Developer/MeshUtilities/Public/MeshUtilities.h"
#include "JsonImporter.h"
#include "JsonObjects/loggers.h"

using namespace UnrealUtilities;
using namespace JsonObjects;

/*
	Amusingly, the most useful file in figuring out how skeletal mesh configuraiton is supposed to work 
*/
void MeshBuilder::setupSkeletalMesh(USkeletalMesh *skelMesh, const JsonMesh &jsonMesh, const JsonImporter *importer, std::function<void(TArray<FSkeletalMaterial> &meshMaterials)> materialSetup){
	check(skelMesh);
	check(importer);
	
	auto importModel = skelMesh->GetImportedModel();
	check(importModel->LODModels.Num() == 0);
	importModel->LODModels.Empty();

	new(importModel->LODModels)FSkeletalMeshLODModel();//????
	//I suppose it does same thing as calling new and then Add()
	auto &lodModel = importModel->LODModels[0];

	auto hasNormals = jsonMesh.normals.Num() != 0;
	auto hasColors = jsonMesh.colors.Num() != 0;
	auto hasTangents = jsonMesh.tangents.Num() != 0;
	auto numTexCoords = jsonMesh.getNumTexCoords();

	skelMesh->bUseFullPrecisionUVs = true;
	skelMesh->bHasVertexColors = hasColors;
	skelMesh->bHasBeenSimplified = false;

	lodModel.NumTexCoords = numTexCoords;

	if (materialSetup){
		materialSetup(skelMesh->Materials);
	}

	auto skelId = jsonMesh.origSkeletonId;
	if (skelId < 0){
		UE_LOG(JsonLog, Warning, TEXT("Skeleton not found on skinned mesh \"%s\"(%d)"), *jsonMesh.name, jsonMesh.id);
		return;
	}
	//auto jsonSkel = 
	auto jsonSkel = importer->getSkeleton(skelId);
	if (!jsonSkel){
		UE_LOG(JsonLog, Warning, TEXT("Json skeleton not found on \"%s\"(%d), skeletonid: %d"), *jsonMesh.name, jsonMesh.id, skelId);
		return;
	}

	auto& refSkeleton = skelMesh->RefSkeleton;
	refSkeleton.Empty();
	{
		FReferenceSkeletonModifier refSkelModifier(refSkeleton, nullptr);

		MatrixArray unityBoneWorldTransforms;
		MatrixArray unrealBoneWorldTransforms;

		unityBoneWorldTransforms.SetNum(jsonSkel->bones.Num());
		unrealBoneWorldTransforms.SetNum(jsonSkel->bones.Num());

		UE_LOG(JsonLog, Log, TEXT("Reconstructing skeleton: %s"), *jsonSkel->name);

		for(int boneIndex = 0; boneIndex < jsonSkel->bones.Num(); boneIndex++){
			UE_LOG(JsonLog, Log, TEXT("Processing bone %d out of %d"), boneIndex, jsonSkel->bones.Num());
			auto &unityWorldMat = unityBoneWorldTransforms[boneIndex];
			auto &unrealWorldMat = unrealBoneWorldTransforms[boneIndex];
			
			unityWorldMat = unrealWorldMat = FMatrix::Identity;

			const auto &srcBone = jsonSkel->bones[boneIndex];

			logValue(TEXT("srcBone.id: "), srcBone.id);
			logValue(TEXT("srcBone.bane: "), srcBone.name);
			logValue(TEXT("srcBone.parentId: "), srcBone.parentId);
			logValue(TEXT("localMatrix: "), srcBone.local);
			logValue(TEXT("worldMatrix: "), srcBone.world);

			auto parentBoneIndex = srcBone.parentId >= 0 ? srcBone.parentId: INDEX_NONE;
			auto boneInfo = FMeshBoneInfo(FName(*srcBone.name), srcBone.name, parentBoneIndex);

			unityWorldMat = srcBone.world;
			unrealWorldMat = unityWorldToUe(unityWorldMat);

			logValue(TEXT("unityWorldMat: "), unityWorldMat);
			logValue(TEXT("unrealWorldMat: "), unrealWorldMat);

			auto unrealLocalMat = unrealWorldMat;

			if (parentBoneIndex >= 0){
				auto parentWorld = unrealBoneWorldTransforms[parentBoneIndex];
				logValue(TEXT("parent: "), parentWorld);
				auto invParent = parentWorld.Inverse();
				logValue(TEXT("invParent: "), invParent);
				unrealLocalMat = unrealLocalMat * invParent;
			}
			logValue(TEXT("unrealLocalMat: "), unrealLocalMat);

			FTransform boneTransform;
			//auto boneTransform = unityWorldToUe(srcBone.pose);
			boneTransform.SetFromMatrix(unrealLocalMat);

			refSkelModifier.Add(boneInfo, boneTransform);
		}
	}

	TArray<SkeletalMeshImportData::FVertInfluence> meshInfluences;
	TArray<SkeletalMeshImportData::FMeshWedge> meshWedges;
	TArray<SkeletalMeshImportData::FMeshFace> meshFaces;
	TArray<FVector> meshPoints;
	TArray<int32> pointToOriginalMap;
	TArray<FText> buildWarnMessages;
	TArray<FName> buildWarnNames;

	const int jsonInfluencesPerVertex = 4;
	for(int vertIndex = 0; vertIndex < jsonMesh.vertexCount; vertIndex++){
		auto srcVert = getIdxVector3(jsonMesh.verts, vertIndex);
		meshPoints.Add(unityPosToUe(srcVert));
		pointToOriginalMap.Add(vertIndex);

		for(int inflIndex = 0; inflIndex < jsonInfluencesPerVertex; inflIndex++){
			auto dataOffset = inflIndex + vertIndex * jsonInfluencesPerVertex;
			auto boneIdx = jsonMesh.boneIndexes[dataOffset]; 
			auto boneWeight = jsonMesh.boneWeights[dataOffset];
			if (boneWeight == 0.0f)
				continue;
			auto &dstInfl = meshInfluences.AddDefaulted_GetRef();
			dstInfl.VertIndex = boneIdx;
			dstInfl.Weight = boneWeight;
			dstInfl.VertIndex = vertIndex;
		}
	}

	//should I just say "screw it" and load it via renderable sections? It would certainly work...
	//Nope, idea dropped. There are issues with renderable sections approach - incorrect center is one of them.

	for(int subMeshIndex = 0; subMeshIndex < jsonMesh.subMeshes.Num(); subMeshIndex++){
		const auto &curSubMesh = jsonMesh.subMeshes[subMeshIndex];
		for(int vertIndexOffset = 0; (vertIndexOffset + 2) < curSubMesh.triangles.Num(); vertIndexOffset += 3){
			auto& dstFace = meshFaces.AddDefaulted_GetRef();
			dstFace.MeshMaterialIndex = subMeshIndex;
			dstFace.SmoothingGroups = 0;

			auto processFaceVertex = [&](int dstFaceIdx, int srcFaceIdx){
				auto srcIdx = vertIndexOffset + srcFaceIdx;

				auto curWedgeIndex = meshWedges.Num();

				auto& dstWedge = meshWedges.AddDefaulted_GetRef();
				auto srcVertIdx = curSubMesh.triangles[srcIdx];
				dstWedge.iVertex = srcVertIdx;

				if (hasColors)
					dstWedge.Color = getIdxColor(jsonMesh.colors, srcVertIdx);

				if (numTexCoords >= 1)
					dstWedge.UVs[0] = unityUvToUnreal(getIdxVector2(jsonMesh.uv0, srcVertIdx));
				if (numTexCoords >= 2)
					dstWedge.UVs[1] = unityUvToUnreal(getIdxVector2(jsonMesh.uv1, srcVertIdx));
				if (numTexCoords >= 3)
					dstWedge.UVs[2] = unityUvToUnreal(getIdxVector2(jsonMesh.uv2, srcVertIdx));
				if (numTexCoords >= 4)
					dstWedge.UVs[3] = unityUvToUnreal(getIdxVector2(jsonMesh.uv3, srcVertIdx));

				processTangent(srcVertIdx, jsonMesh.normals, jsonMesh.tangents, hasNormals, hasTangents, 
					[&](const auto &norm){
						dstFace.TangentZ[dstFaceIdx] = norm;
					},
					[&](const auto &tanU, const auto &tanV){
						dstFace.TangentX[dstFaceIdx] = tanU;
						dstFace.TangentY[dstFaceIdx] = tanV;
					}
				);

				dstFace.iWedge[dstFaceIdx] = curWedgeIndex;
			};

			processFaceVertex(0, 0);
			processFaceVertex(1, 2);
			processFaceVertex(2, 1);
		}
	}

	IMeshUtilities::MeshBuildOptions buildOptions;
	buildOptions.bComputeNormals = !hasNormals;
	buildOptions.bComputeTangents = !hasTangents;//true;
	IMeshUtilities& meshUtils = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	meshUtils.BuildSkeletalMesh(lodModel, 
		refSkeleton, 
		meshInfluences, meshWedges, meshFaces, meshPoints, 
		pointToOriginalMap, buildOptions, &buildWarnMessages, &buildWarnNames);

	for(auto warn: buildWarnMessages){
		UE_LOG(JsonLog, Warning, TEXT("Warning message: %s"), *warn.ToString());
	}
	for(const auto& warn: buildWarnNames){
		UE_LOG(JsonLog, Warning, TEXT("Warning name: %s"), *warn.ToString());
	}

	auto skeleton = NewObject<USkeleton>(skelMesh->GetOuter());
	skeleton->MergeAllBonesToBoneTree(skelMesh);
	skelMesh->Skeleton = skeleton;
	FAssetRegistryModule::AssetCreated(skeleton);
	skeleton->MarkPackageDirty();

	skelMesh->PostEditChange();
	skelMesh->MarkPackageDirty();
	skelMesh->PostLoad();
}
