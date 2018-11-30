#include "JsonImportPrivatePCH.h"
#include "MeshBuilder.h"
#include "UnrealUtilities.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshModel.h"
#include "Developer/MeshUtilities/Public/MeshUtilities.h"
#include "JsonImporter.h"

using namespace UnrealUtilities;

/*
	Amusingly, the most useful file in figuring out how skeletal mesh configuraiton is supposed to work 
*/
void MeshBuilder::setupSkeletalMesh(USkeletalMesh *mesh, const JsonMesh &jsonMesh, const JsonImporter *importer, std::function<void(TArray<FSkeletalMaterial> &meshMaterials)> materialSetup){
	check(mesh);
	check(importer);
	
	auto importModel = mesh->GetImportedModel();
	check(importModel->LODModels.Num() == 0);
	importModel->LODModels.Empty();

	new(importModel->LODModels)FSkeletalMeshLODModel();//????
	//I suppose it does same thing as calling new and then Add()
	auto &lodModel = importModel->LODModels[0];
	auto &lodInfo = mesh->AddLODInfo();

	if (materialSetup){
		materialSetup(mesh->Materials);
	}

	for (int i = 0; i < mesh->Materials.Num(); i++){
		lodInfo.LODMaterialMap.Add(i);
	}

	auto& refSkeleton = mesh->RefSkeleton;
	refSkeleton.Empty();

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

	FReferenceSkeletonModifier refSkelModifier(refSkeleton, nullptr);

	for(int boneIndex = 0; boneIndex < jsonSkel->bones.Num(); boneIndex++){
		lodModel.RequiredBones.Add(boneIndex);
		lodModel.ActiveBoneIndices.Add(boneIndex);

		const auto &srcBone = jsonSkel->bones[boneIndex];
		FName boneName = *srcBone.name;

		 auto boneInfo = FMeshBoneInfo(FName(*srcBone.name), srcBone.name, srcBone.parent);

		auto unityPose = srcBone.pose;
		auto unrealPose = unityWorldToUe(unityPose);
		FTransform boneTransform;
		//auto boneTransform = unityWorldToUe(srcBone.pose);
		boneTransform.SetFromMatrix(unrealPose);

		refSkelModifier.Add(boneInfo, boneTransform);
	}

	auto hasNormals = jsonMesh.normals.Num() != 0;
	auto hasColors = jsonMesh.colors.Num() != 0;
	auto hasTangents = jsonMesh.tangents.Num() != 0;
	auto numTexCoords = jsonMesh.getNumTexCoords();

	TArray<SkeletalMeshImportData::FVertInfluence> meshInfluences;
	TArray<SkeletalMeshImportData::FMeshWedge> meshWedges;
	TArray<SkeletalMeshImportData::FMeshFace> meshFaces;
	TArray<FVector> meshPoints;
	TArray<int32> pointToOriginalMap;
	TArray<FText> buildWarnMessages;
	TArray<FName> buildWarnNames;

	for(int i = 0; i < jsonMesh.vertexCount; i++){
		auto srcVert = getIdxVector3(jsonMesh.verts, i);
		meshPoints.Add(unityPosToUe(srcVert));
		pointToOriginalMap.Add(i);
	}

	//should I just say "screw it" and load it via renderable sections? It would certainly work...

	//influences?
	for(int subMeshIndex = 0; subMeshIndex < jsonMesh.subMeshes.Num(); subMeshIndex++){
		const auto &curSubMesh = jsonMesh.subMeshes[subMeshIndex];
		for(int faceVertOffset = 0; (faceVertOffset + 2) < curSubMesh.triangles.Num(); faceVertOffset += 3){
			auto& dstFace = meshFaces.AddDefaulted_GetRef();
			dstFace.MeshMaterialIndex = subMeshIndex;

			auto processFaceVertex = [&](int faceVertDstIdx, int faceVertSrcIdx){
				auto srcIdx = faceVertOffset + faceVertSrcIdx;

				auto curWedgeIndex = meshWedges.Num();

				auto& dstWedge = meshWedges.AddDefaulted_GetRef();
				//dstWedge.MatIndex = subMeshIndex;
				auto srcVertIdx = curSubMesh.triangles[srcIdx];
				//dstWedge.VertexIndex = srcVertIdx;
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

				///this needs to be moved into a subroutine...

				processTangent(srcVertIdx, jsonMesh.normals, jsonMesh.tangents, hasNormals, hasTangents, 
					[&](const auto &norm){
						dstFace.TangentZ[faceVertDstIdx] = norm;
					},
					[&](const auto &tanU, const auto &tanV){
						dstFace.TangentX[faceVertDstIdx] = tanU;
						dstFace.TangentY[faceVertDstIdx] = tanV;
					}
				);

				//dstFace.WedgeIndex[faceVertDstIdx] = curWedgeIndex;
				dstFace.iWedge[faceVertDstIdx] = curWedgeIndex;
			};

			processFaceVertex(0, 0);
			processFaceVertex(1, 2);
			processFaceVertex(2, 1);
		}
	}

	mesh->Skeleton = NewObject<USkeleton>();
	mesh->Skeleton->MergeAllBonesToBoneTree(mesh);
	mesh->PostLoad();

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

#if 0
#endif

	/*
	meshUtils.BuildSkeletalMesh( 
		*NewModel, RefSkeleton, MeshData.Influences, MeshData.Wedges, MeshData.Faces, MeshData.Points, DummyMap, Options);*/

	//NewModel->NumTexCoords = MeshData.TexCoordCount;

	/*
	if (materialSetup){
		materialSetup(mesh->Materials);
	}

	*/
}

void MeshBuilder::setupSkeleton(USkeleton *skeleton, const JsonSkeleton &jsonSkel){
}
