#include "JsonImportPrivatePCH.h"
#include "MeshBuilder.h"
#include "UnrealUtilities.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshModel.h"
#include "Developer/MeshUtilities/Public/MeshUtilities.h"
#include "JsonImporter.h"
#include "JsonObjects/loggers.h"

#include "Runtime/Engine/Classes/Animation/MorphTarget.h"
#include "Runtime/Engine/Classes/Animation/Skeleton.h"
#include "Runtime/Engine/Classes/Engine/PreviewMeshCollection.h"

using namespace UnrealUtilities;
using namespace JsonObjects;


/*
	Amusingly, the most useful file in figuring out how skeletal mesh configuraiton is supposed to work 
*/
void MeshBuilder::setupSkeletalMesh(USkeletalMesh *skelMesh, const JsonMesh &jsonMesh, const JsonImporter *importer, std::function<void(TArray<FSkeletalMaterial> &meshMaterials)> materialSetup, std::function<void(const JsonSkeleton&, USkeleton*)> onNewSkeleton){
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

	auto skelId = jsonMesh.defaultSkeletonId;
	if (skelId < 0){
		UE_LOG(JsonLog, Warning, TEXT("Skeleton not found on skinned mesh \"%s\"(%d)"), *jsonMesh.name, jsonMesh.id);
		return;
	}

	auto jsonSkel = importer->getSkeleton(skelId);
	if (!jsonSkel){
		UE_LOG(JsonLog, Warning, TEXT("Json skeleton not found on \"%s\"(%d), skeletonid: %d"), *jsonMesh.name, jsonMesh.id, skelId);
		return;
	}

	TMap<int, int> meshToSkeletonBoneMap;
	for(int boneIndex = 0; boneIndex < jsonMesh.defaultBoneNames.Num(); boneIndex++){
		const auto &curName = jsonMesh.defaultBoneNames[boneIndex];
		const auto skeletonBoneIndex = jsonSkel->findBoneIndex(curName);
		if (skeletonBoneIndex < 0){
			UE_LOG(JsonLog, Warning, TEXT("Bone \"%s\" not found while processing mesh \"%s\""), 
				*curName, *jsonMesh.name);
			continue;
		}
		meshToSkeletonBoneMap.Add(boneIndex, skeletonBoneIndex);
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
			//UE_LOG(JsonLog, Log, TEXT("Processing bone %d out of %d"), boneIndex, jsonSkel->bones.Num());
			auto &unityWorldMat = unityBoneWorldTransforms[boneIndex];
			auto &unrealWorldMat = unrealBoneWorldTransforms[boneIndex];
			
			unityWorldMat = unrealWorldMat = FMatrix::Identity;

			const auto &srcBone = jsonSkel->bones[boneIndex];

			/*
			logValue(TEXT("srcBone.id: "), srcBone.id);
			logValue(TEXT("srcBone.bane: "), srcBone.name);
			logValue(TEXT("srcBone.parentId: "), srcBone.parentId);
			logValue(TEXT("localMatrix: "), srcBone.local);
			logValue(TEXT("worldMatrix: "), srcBone.world);
			*/

			auto parentBoneIndex = srcBone.parentId >= 0 ? srcBone.parentId: INDEX_NONE;
			auto boneInfo = FMeshBoneInfo(FName(*srcBone.name), srcBone.name, parentBoneIndex);

			unityWorldMat = srcBone.world;
			unrealWorldMat = unityWorldToUe(unityWorldMat);

			//logValue(TEXT("unityWorldMat: "), unityWorldMat);
			//logValue(TEXT("unrealWorldMat: "), unrealWorldMat);

			auto unrealLocalMat = unrealWorldMat;

			if (parentBoneIndex >= 0){
				auto parentWorld = unrealBoneWorldTransforms[parentBoneIndex];
				//logValue(TEXT("parent: "), parentWorld);
				auto invParent = parentWorld.Inverse();
				//logValue(TEXT("invParent: "), invParent);
				unrealLocalMat = unrealLocalMat * invParent;
			}
			//logValue(TEXT("unrealLocalMat: "), unrealLocalMat);

			FTransform boneTransform;
			//auto boneTransform = unityWorldToUe(srcBone.pose);
			boneTransform.SetFromMatrix(unrealLocalMat);

			refSkelModifier.Add(boneInfo, boneTransform);
		}
	}

	TArray<UMorphTarget*> morphTargets;

	TArray<SkeletalMeshImportData::FVertInfluence> meshInfluences;
	TArray<SkeletalMeshImportData::FMeshWedge> meshWedges;
	TArray<SkeletalMeshImportData::FMeshFace> meshFaces;
	TArray<FVector> meshPoints;
	TArray<int32> pointToOriginalMap;
	TArray<FText> buildWarnMessages;
	TArray<FName> buildWarnNames;

	const int jsonInfluencesPerVertex = 4;

	TArray<FString> remapErrors;
	//Yes, it is possible to get "skinned" mesh with no bones. It will have blendshapes only.
	bool hasBones = jsonMesh.boneIndexes.Num() > 0;
	for(int vertIndex = 0; vertIndex < jsonMesh.vertexCount; vertIndex++){
		auto srcVert = getIdxVector3(jsonMesh.verts, vertIndex);
		meshPoints.Add(unityPosToUe(srcVert));
		pointToOriginalMap.Add(vertIndex);

		if (hasBones){
			for(int inflIndex = 0; inflIndex < jsonInfluencesPerVertex; inflIndex++){
				auto dataOffset = inflIndex + vertIndex * jsonInfluencesPerVertex;
				auto meshBoneIdx = jsonMesh.boneIndexes[dataOffset]; 
				auto boneWeight = jsonMesh.boneWeights[dataOffset];
				#if 0
				/*if (boneWeight == 0.0f)
					continue;*/
				#endif
				auto &dstInfl = meshInfluences.AddDefaulted_GetRef();
				auto skelBoneIdx = meshBoneIdx;
				auto foundIdx = meshToSkeletonBoneMap.Find(meshBoneIdx);
				if (!foundIdx){
					remapErrors.Add(
						FString::Printf(TEXT("Could not remap mesh bone index %d in vertex influence, errors are possible"),
							meshBoneIdx));
					/*
					UE_LOG(JsonLog, Error, TEXT("Could not remap mesh bone index %d in vertex influence, errors are possible"),
						meshBoneIdx);*/
				}
				else{
					skelBoneIdx = *foundIdx;
				}

				dstInfl.BoneIndex = skelBoneIdx;//meshBoneIdx;
				dstInfl.Weight = boneWeight;
				dstInfl.VertIndex = vertIndex;
			}
		}
	}

	if (remapErrors.Num()){
		FString combinedMessage = FString::Printf(TEXT("Remap errors found while processing skeletal mesh %d(\"%s\")\n"), jsonMesh.id, *jsonMesh.name);
		for(const auto& cur: remapErrors){
			combinedMessage += FString::Printf(TEXT("%s\n"), *cur);
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

	if (buildWarnMessages.Num() || buildWarnNames.Num()){
		FString msg;
		msg += TEXT("Warnings:\n");
		for(auto warn: buildWarnMessages){
			msg += FString::Printf(TEXT("%s\n"), *warn.ToString());
		}
		msg += TEXT("Warning names:\n");
		for(const auto& warn: buildWarnNames){
			msg += FString::Printf(TEXT("%s\n"), *warn.ToString());
		}
		UE_LOG(JsonLog, Warning, TEXT("Warning while building skeletal mesh %d(\"%s\"):%s"), jsonMesh.id, *jsonMesh.name, *msg);
	}
	/*
	for(auto warn: buildWarnMessages){
		UE_LOG(JsonLog, Warning, TEXT("Warning message: %s"), *warn.ToString());
	}
	for(const auto& warn: buildWarnNames){
		UE_LOG(JsonLog, Warning, TEXT("Warning name: %s"), *warn.ToString());
	}
	*/

	auto newSkelName = FString::Printf(TEXT("%s_%d"), *jsonSkel->name, jsonSkel->id);

	auto foundSkeleton = importer->getSkeletonObject(jsonSkel->id);
	if (!foundSkeleton){
		auto desiredDir = FPaths::GetPath(jsonMesh.path);
		auto skelName = FString::Printf(TEXT("%s_%s_%d"), *jsonSkel->name, TEXT("skel"), jsonSkel->id);
		auto skeleton = createAssetObject<USkeleton>(
			skelName, &desiredDir, importer, 
			[&](auto arg){
				arg->MergeAllBonesToBoneTree(skelMesh);
			}, RF_Standalone|RF_Public
		);
		if (onNewSkeleton)
			onNewSkeleton(*jsonSkel, skeleton);
		skelMesh->Skeleton = skeleton;
	}
	else{
		skelMesh->Skeleton = foundSkeleton;
	}

	auto collectionSkel = skelMesh->Skeleton;

	bool needMorphInvalidate = false;

	for(int blendShapeIndex = 0; blendShapeIndex < jsonMesh.blendShapes.Num(); blendShapeIndex++){
		UE_LOG(JsonLog, Log, TEXT("Processing blend shape %d out of %d"), blendShapeIndex, jsonMesh.blendShapes.Num());
		const auto &curBlendShape = jsonMesh.blendShapes[blendShapeIndex];
		for(int blendFrameIndex = 0; blendFrameIndex < curBlendShape.frames.Num(); blendFrameIndex++){
			auto morphName = FString::Printf(TEXT("%s_%s_s%d_f%d"), 
				*jsonMesh.name, *curBlendShape.name, blendShapeIndex, blendFrameIndex);

			const auto &blendFrame = curBlendShape.frames[blendFrameIndex];

			auto morphTarget = NewObject<UMorphTarget>(skelMesh->GetOuter(), *morphName);
			FAssetRegistryModule::AssetCreated(morphTarget);
			morphTargets.Add(morphTarget);

			TArray<FMorphTargetDelta> deltas;

			auto importData = skelMesh->GetImportedModel();
			const auto &lodModel = importData->LODModels[0];//TODO:  lod support.

			FString modelData;

			//UE_LOG(JsonLog, Log, TEXT("Dumping model: %s"), *modelData);

			for(int meshVertIdx = 0; meshVertIdx < lodModel.MeshToImportVertexMap.Num(); meshVertIdx++){
				auto origVertIdx = lodModel.MeshToImportVertexMap[meshVertIdx];

				auto& dstDelta = deltas.AddDefaulted_GetRef();
				dstDelta.SourceIdx = meshVertIdx;//Aaand nope. That was some stupid error. Mesh morph references the NEW verts, and not the original one...
				auto unityDeltaPos = getIdxVector3(blendFrame.deltaVerts, origVertIdx);
				auto unityDeltaNorm = getIdxVector3(blendFrame.deltaNormals, origVertIdx);

				dstDelta.PositionDelta = unityPosToUe(unityDeltaPos);
				dstDelta.TangentZDelta = unityVecToUe(unityDeltaNorm);
			}
			//lodModel.MeshToImportVertexMap
			//UE_LOG(JsonLog, Log, TEXT("Printing original vertices. Total number: %d"), lodModel.RawPointIndices.Nu

			morphTarget->PopulateDeltas(deltas, 0, lodModel.Sections);

			morphTarget->MarkPackageDirty();

			auto registrationResult = skelMesh->RegisterMorphTarget(morphTarget);
			needMorphInvalidate = needMorphInvalidate | registrationResult;
			UE_LOG(JsonLog, Log, TEXT("Registration result: %d. Target %s (%d), frame %d"),
				(int)registrationResult, *curBlendShape.name, blendShapeIndex, blendFrameIndex);
		}
	}

	if (needMorphInvalidate){
		skelMesh->InitMorphTargetsAndRebuildRenderData();
	}

	//Bounding box calculation. Failure to do that causes the model to flicker. 
	FBox boundBox(meshPoints.GetData(), meshPoints.Num());
	auto tmpBox = boundBox;
	auto midPoint = (boundBox.Min + boundBox.Max) * 0.5f;

	//This is done in skeletal mesh import in fbx. Apparnetly it doubles boundbox size, except it raises bottom of the box to the leg level.
	boundBox.Min = tmpBox.Min + 1.0f*(tmpBox.Min - midPoint);
	boundBox.Max = tmpBox.Max + 1.0f*(tmpBox.Max - midPoint);
	boundBox.Min[2] = tmpBox.Min[2] + 0.1f*(tmpBox.Min[2] - midPoint[2]);

	skelMesh->SetImportedBounds(FBoxSphereBounds(boundBox));

	/*
	Here's the thing thoug h - Since we're operating on unity skeletons, the data will not behave the same way Unreal FBX importer expects it.

	Meaning, there may be a piece of clothing floating above the ground. 

	Hmm.
	*/

	registerPreviewMesh(skelMesh->Skeleton, skelMesh, jsonMesh);


	skelMesh->PostEditChange();
	skelMesh->MarkPackageDirty();
	skelMesh->PostLoad();
}

void MeshBuilder::registerPreviewMesh(USkeleton *skel, USkeletalMesh *mesh, const JsonMesh &jsonMesh){
	check(skel);
	check(mesh);

	auto previewMesh = skel->GetPreviewMesh();
	auto collectionAsset = skel->GetAdditionalPreviewSkeletalMeshes();

	//That's our mesh.
	if (previewMesh == mesh)
		return;
	
	UE_LOG(JsonLog, Log, TEXT("Preview mesh: %x; collection: %x"), previewMesh, collectionAsset);
	//collection needed;
	if (!previewMesh){
		skel->SetPreviewMesh(mesh);
		UE_LOG(JsonLog, Log, TEXT("Preview mesh set"));
		UE_LOG(JsonLog, Log, TEXT("Preview mesh: %x; collection: %x"), skel->GetPreviewMesh(), skel->GetAdditionalPreviewSkeletalMeshes());
		skel->MarkPackageDirty();
		skel->PostEditChange();
		return;
	}

	auto collection = Cast<UPreviewMeshCollection>(collectionAsset);

	if (!collection){
		auto desiredPath = FPaths::GetPath(jsonMesh.path);
		check(skel);
		auto skelPath = skel->GetPathName();

		auto collectionPath = skelPath + TEXT("_collection");
		
		collection = createAssetObject<UPreviewMeshCollection>(skelPath, &desiredPath, nullptr, nullptr, RF_Standalone|RF_Public);
		UE_LOG(JsonLog, Log, TEXT("Collection created: %x"), collection);
		check(collection);
		skel->SetAdditionalPreviewSkeletalMeshes(collection);
	}

	for(const auto& cur: collection->SkeletalMeshes){
		if (cur.SkeletalMesh == mesh){
			UE_LOG(JsonLog, Log, TEXT("Skeletal mesh %d(%s) is already a part of collection"), jsonMesh.id, *jsonMesh.name);
			return;
		}
	}

	auto& newVal = collection->SkeletalMeshes.AddDefaulted_GetRef();
	newVal.SkeletalMesh = mesh;

	collection->MarkPackageDirty();
	collection->PostEditChange();

	//collection->Mesh


}


USkeleton* JsonImporter::getSkeletonObject(int32 id) const{
	auto found = skeletonIdMap.Find(id);
	if (!found)
		return nullptr;
	return LoadObject<USkeleton>(nullptr, **found);
}

void JsonImporter::registerSkeleton(int32 id, USkeleton *skel){
	check(skel);
	check(id >= 0);

	if (skeletonIdMap.Contains(id)){
		UE_LOG(JsonLog, Log, TEXT("Duplicate skeleton registration for id %d"), id);
		return;
	}

	auto path = skel->GetPathName();
	skeletonIdMap.Add(id, path);
	//auto outer = skel->
}
