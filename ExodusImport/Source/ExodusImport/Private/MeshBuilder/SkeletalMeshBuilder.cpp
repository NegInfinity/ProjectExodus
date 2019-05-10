#include "JsonImportPrivatePCH.h"
#include "SkeletalMeshBuilder.h"
#include "MeshBuilder.h"
#include "MeshBuilderUtils.h"
#include "UnrealUtilities.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshModel.h"
#include "Developer/MeshUtilities/Public/MeshUtilities.h"
#include "JsonImporter.h"
#include "JsonObjects/loggers.h"
#include "AssetRegistryModule.h"

#include "Runtime/Engine/Classes/Animation/MorphTarget.h"
#include "Runtime/Engine/Classes/Animation/Skeleton.h"
#include "Runtime/Engine/Classes/Engine/PreviewMeshCollection.h"

using namespace UnrealUtilities;
using namespace JsonObjects;
using namespace MeshBuilderUtils;

void SkeletalMeshBuilder::setupReferenceSkeleton(FReferenceSkeleton &refSkeleton, const JsonSkeleton &jsonSkel, const JsonMesh *jsonMesh, const USkeleton *unrealSkeleton) const{
	refSkeleton.Empty();
	FReferenceSkeletonModifier refSkelModifier(refSkeleton, unrealSkeleton);//nullptr);

	MatrixArray unityBoneWorldTransforms;
	MatrixArray unrealBoneWorldTransforms;

	unityBoneWorldTransforms.SetNum(jsonSkel.bones.Num());
	unrealBoneWorldTransforms.SetNum(jsonSkel.bones.Num());

	UE_LOG(JsonLog, Log, TEXT("Reconstructing skeleton: %s"), *jsonSkel.name);

	for(int boneIndex = 0; boneIndex < jsonSkel.bones.Num(); boneIndex++){
		//UE_LOG(JsonLog, Log, TEXT("Processing bone %d out of %d"), boneIndex, jsonSkel->bones.Num());
		auto &unityWorldMat = unityBoneWorldTransforms[boneIndex];
		auto &unrealWorldMat = unrealBoneWorldTransforms[boneIndex];
			
		unityWorldMat = unrealWorldMat = FMatrix::Identity;

		const auto &srcBone = jsonSkel.bones[boneIndex];

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

		if (jsonMesh){
			auto foundBoneIndex = jsonMesh->defaultBoneNames.IndexOfByKey(srcBone.name);
			UE_LOG(JsonLog, Log, TEXT("Attempting to find foundBoneIndex for bone \"%s\" on mesh %d(\"%s\")"), 
				*srcBone.name, jsonMesh->id.toIndex(), *jsonMesh->name);
			if (foundBoneIndex != INDEX_NONE){
				auto boneMatrix = jsonMesh->inverseBindPoses[foundBoneIndex];
				UE_LOG(JsonLog, Log, TEXT("Bone index found: %d"), foundBoneIndex);
				logValue(TEXT("Matrix: "), boneMatrix);
				unityWorldMat = boneMatrix;
			}
			else{
				UE_LOG(JsonLog, Warning, TEXT("Bone \"%s\" not found"), *srcBone.name);
			}
		}

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

void SkeletalMeshBuildData::processWedgeData(const JsonMesh &jsonMesh){
	const auto numTexCoords = jsonMesh.getNumTexCoords();

	check(hasNormals == (jsonMesh.normals.Num() != 0));
	check(hasTangents == (jsonMesh.tangents.Num() != 0));
	check(hasColors == (jsonMesh.colors.Num() != 0));
	/*
	auto hasNormals = jsonMesh.normals.Num() != 0;
	auto hasTangents = jsonMesh.tangents.Num() != 0;
	auto hasColors = jsonMesh.colors.Num() != 0;
	*/

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
}

void SkeletalMeshBuildData::startWithMesh(const JsonMesh &jsonMesh){
	hasNormals = jsonMesh.normals.Num() != 0;
	hasTangents = jsonMesh.tangents.Num() != 0;
	hasColors = jsonMesh.colors.Num() != 0;
}

void SkeletalMeshBuildData::buildSkeletalMesh(FSkeletalMeshLODModel &lodModel, const FReferenceSkeleton &refSkeleton, const JsonMesh &jsonMesh){
	IMeshUtilities::MeshBuildOptions buildOptions;
	buildOptions.bComputeNormals = !hasNormals;
	buildOptions.bComputeTangents = !hasTangents;//true;

	/*
	buildOptions.OverlappingThresholds.ThresholdPosition = 0.0f;
	buildOptions.OverlappingThresholds.ThresholdTangentNormal = 0.0f;
	buildOptions.OverlappingThresholds.ThresholdUV = 0.0f;
	*/

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
		UE_LOG(JsonLog, Warning, TEXT("Warning while building skeletal mesh %d(\"%s\"):%s"), (int)jsonMesh.id, *jsonMesh.name, *msg);
	}
}

void SkeletalMeshBuildData::computeBoundingBox(USkeletalMesh *skelMesh, const JsonMesh &jsonMesh){
	//Bounding box calculation. Failure to do that causes the model to flicker. 
	FBox boundBox(meshPoints.GetData(), meshPoints.Num());
	auto tmpBox = boundBox;
	auto midPoint = (boundBox.Min + boundBox.Max) * 0.5f;
	UE_LOG(JsonLog, Log, TEXT("Bounding Box(%d:\"%s\") Min: %f %f %f ; Max: %f %f %f"),
		(int)jsonMesh.id, *jsonMesh.name,
		tmpBox.Min.X, tmpBox.Min.Y, tmpBox.Min.Z, tmpBox.Max.X, tmpBox.Max.Y, tmpBox.Max.Z);

	//This is done in skeletal mesh import in fbx. Apparnetly it doubles boundbox size, except it raises bottom of the box to the leg level.
	boundBox.Min = tmpBox.Min + 1.0f*(tmpBox.Min - midPoint);
	boundBox.Max = tmpBox.Max + 1.0f*(tmpBox.Max - midPoint);
	boundBox.Min[2] = tmpBox.Min[2] + 0.1f*(tmpBox.Min[2] - midPoint[2]);

	skelMesh->SetImportedBounds(FBoxSphereBounds(boundBox));
}


struct SkeletalMeshInfluence{
	int boneIndex = 0;
	float weight = 0.0f;
	int intWeight = 0;
	void recomputeInt(){
		intWeight = (int)(255.0f * weight);
	}
	void recomputeFloat(){
		weight = ((float)intWeight)/255.0f;
	}
	SkeletalMeshInfluence() = default;
	SkeletalMeshInfluence(int boneIndex_, float weight_)
	:boneIndex(boneIndex_), weight(weight_){
		intWeight = (int)(weight * 255.0f);//truncate
	}
};

using SkeletalMeshInfluenceArray = TArray<SkeletalMeshInfluence>;
using BoneInfluenceMap = TMap<int, SkeletalMeshInfluenceArray>;

float getTotalWeight(const SkeletalMeshInfluenceArray &arr){
	float result = 0.0f;
	for(const auto &cur: arr){
		result += cur.weight;
	}
	return result;
}

int getTotalIntWeight(const SkeletalMeshInfluenceArray &arr){
	int result = 0;
	for(const auto &cur: arr){
		result += cur.intWeight;
	}
	return result;
}

void printBoneInfluenceMap(const BoneInfluenceMap &boneInfluences){
	for(auto &cur: boneInfluences){
		UE_LOG(JsonLog, Log, TEXT("Influence for vert %d: %f (%d)"), cur.Key, getTotalWeight(cur.Value), getTotalIntWeight(cur.Value));
		for(int i = 0; i < cur.Value.Num(); i++){
			UE_LOG(JsonLog, Log, TEXT("Influence %d/%d: %f (%d)"), i, cur.Value.Num(), cur.Value[i].weight, cur.Value[i].intWeight);
		}
	}
}

void normalizeInfluenceMap(BoneInfluenceMap &boneInfluences){
	for(auto &cur: boneInfluences){
		if (cur.Value.Num() == 0)
			continue;

		for(auto &infl: cur.Value){
			infl.weight = FMath::Clamp(infl.weight, 0.0f, 1.0f);
			infl.recomputeInt();
		}

		float totalFloat = getTotalWeight(cur.Value);
		if ((totalFloat > 1.0f) && (totalFloat != 0.0f)){
			float scale = 1.0f/totalFloat;
			for(auto &infl: cur.Value){
				infl.weight *= scale;
				infl.recomputeInt();
			}
		}

		int totalInt = getTotalIntWeight(cur.Value);
		check((totalInt >= 0) && (totalInt <= 255));//This shouldn't fire at this point, but you never know.

		auto extra = 255 - totalInt;
		if (extra > 0){
			auto& largest = cur.Value[0];
			largest.intWeight += extra;
			largest.recomputeFloat();
		}

		for(auto &infl: cur.Value){
			infl.recomputeFloat();
		}
	}
}

void SkeletalMeshBuildData::processPositionsAndWeights(const JsonMesh &jsonMesh, const TMap<int, int> &meshToSkeletonBoneMap, StringArray &remapErrors){
//void SkeletalMeshBuildData::processVerts(const JsonMesh &jsonMesh, StringArray &remapErrors){
	const int jsonInfluencesPerVertex = 4;

	bool hasBones = jsonMesh.boneIndexes.Num() > 0;

	/*
	//TMap<int, float> summaryWeightMap;
	//TMap<int, TArray<int>> influenceIndexes;

	//TMap<int, int> summaryIntWeightMap;
	*/

	//vertices themselves
	for(int vertIndex = 0; vertIndex < jsonMesh.vertexCount; vertIndex++){
		auto srcVert = getIdxVector3(jsonMesh.verts, vertIndex);
		meshPoints.Add(unityPosToUe(srcVert));
		pointToOriginalMap.Add(vertIndex);
	}

	/*
	the mapping to uint8 for bone weights is troublesome.
	*/
	TMap<int, SkeletalMeshInfluenceArray> boneInfluences;
	if (hasBones){
		for(int vertIndex = 0; vertIndex < jsonMesh.vertexCount; vertIndex++){
			for(int inflIndex = 0; inflIndex < jsonInfluencesPerVertex; inflIndex++){
				auto dataOffset = inflIndex + vertIndex * jsonInfluencesPerVertex;
				auto meshBoneIdx = jsonMesh.boneIndexes[dataOffset]; 
				auto boneWeight = jsonMesh.boneWeights[dataOffset];
				//if (boneWeight < 0.0f)//There actually ARE negative weight somewhere, andd they cause mesh spikes.
				if (boneWeight <= 0.0f)
					continue;

				//auto dstInfluenceIndex = meshInfluences.Num();
				auto skelBoneIdx = meshBoneIdx;
				auto foundIdx = meshToSkeletonBoneMap.Find(meshBoneIdx);
				if (!foundIdx){
					remapErrors.Add(
						FString::Printf(TEXT("Could not remap mesh bone index %d in vertex influence, errors are possible"),
							meshBoneIdx));
				}
				else{
					skelBoneIdx = *foundIdx;
				}

				SkeletalMeshInfluence tmpInfluence(skelBoneIdx, boneWeight);
				boneInfluences.FindOrAdd(vertIndex).Add(tmpInfluence);
			}
		}
	}
	else{
		UE_LOG(JsonLog, Log, TEXT("The mesh \"%s\"(%d) has no bones. Remapping it to the original parent \"%s\""),
			*jsonMesh.name, (int)jsonMesh.id, *jsonMesh.defaultMeshNodeName);
		//well. We're remapping it to the single bone the skeleton has. 
		int origIndex = 0;//yep. Always a bone 0.
		auto foundIdx = meshToSkeletonBoneMap.Find(origIndex);
		auto remappedIndex = origIndex;
		if (!foundIdx){
			remapErrors.Add(
				FString::Printf(TEXT("Could not remap mesh bone index %d in vertex influence, errors are possible"),
					origIndex));
		}
		else{
			remappedIndex = *foundIdx;
		}
		for(int vertIndex = 0; vertIndex < jsonMesh.vertexCount; vertIndex++){
			SkeletalMeshInfluence tmpInfluence(remappedIndex, 1.0f);
			boneInfluences.FindOrAdd(vertIndex).Add(tmpInfluence);
		}
	}

	/*
	UE_LOG(JsonLog, Log, TEXT("Pre-sort bone influences on %s(%d)"), *jsonMesh.name, jsonMesh.id);
	printBoneInfluenceMap(boneInfluences);
	*/

	//Sorting the influences from strongest to weakest
	for(auto &cur: boneInfluences){
		Algo::SortBy(cur.Value, [](const SkeletalMeshInfluence &arg){return -arg.weight;});
		//cur.Value.Sort(
	}

	/*
	UE_LOG(JsonLog, Log, TEXT("Post-sort bone influences on %s(%d)"), *jsonMesh.name, jsonMesh.id);
	printBoneInfluenceMap(boneInfluences);
	*/

	normalizeInfluenceMap(boneInfluences);

	/*
	UE_LOG(JsonLog, Log, TEXT("Post-normalization bone influences on %s(%d)"), *jsonMesh.name, jsonMesh.id);
	printBoneInfluenceMap(boneInfluences);
	*/

	for(const auto &cur: boneInfluences){
		float total = 0.0f;
		for(const auto &infl: cur.Value){
			auto &dstInfl = meshInfluences.AddDefaulted_GetRef();
			dstInfl.VertIndex = cur.Key;
			dstInfl.BoneIndex = infl.boneIndex;//meshBoneIdx;
			dstInfl.Weight = infl.weight;// * 2.0f;
			total += infl.weight;

			if (total >= 256.0f/255.0f){
				UE_LOG(JsonLog, Log, TEXT("Total overflow: %f on vertex %d"), total, dstInfl.VertIndex);
			}
			//dstInfl.Weight = 1.0f;
			//
			//break;
		}
	}

	/*
	for(auto& cur: boneInfluences){
		auto &influences = cur.Value;
		
		auto totalInt = getTotalIntWeight(influences);
		auto totalFloat = getTotalWeight(influences);
		if (totalFloat != 1.0f){
			UE_LOG(JsonLog, Warning, TEXT("Invalid total bone on vertex %d, mesh %s (%d), value %f"),
				cur.Key, *jsonMesh.name, jsonMesh.id, totalFloat);
		}
		if (totalInt != 255){
			UE_LOG(JsonLog, Warning, TEXT("Invalid total int bone on vertex %d, mesh %s (%d), value %d"),
				cur.Key, *jsonMesh.name, jsonMesh.id, totalInt);
			if (totalInt > 255){
				UE_LOG(JsonLog, Warning, TEXT("Value is too large and will result in wraparound!"));
			}
		}
	}
	*/

	/*
	for(const auto &cur: summaryIntWeightMap){
		if ((cur.Value > 255) || (cur.Value < 0)){
			UE_LOG(JsonLog, Warning, TEXT("Invalid total int weight %d on vertex %d, mesh %s (%d)"),
				cur.Value, cur.Key, *jsonMesh.name, jsonMesh.id)
		}
	}

	for(int vertIndex = 0; vertIndex < jsonMesh.vertexCount; vertIndex++){
		if (!summaryWeightMap.Contains(vertIndex)){
			UE_LOG(JsonLog, Warning, TEXT("Unbound skin vertex %d on mesh %s(%d)"), vertIndex, *jsonMesh.name, jsonMesh.id);
		}
	}
	*/

	/*
	for(const auto &cur: summaryWeightMap){
		auto &curIndexes = cur.Value;
		for(const auto influeneIndexes: curIndexes){
		}
	}
	*/
	/*
	for(const auto &cur: summaryWeightMap){
		if (cur.Value > 1.0f){
			UE_LOG(JsonLog, Warning, TEXT("Bone weight value %f is too high on vertex %d, mesh %s (%d)"),
				cur.Value, cur.Key, *jsonMesh.name, jsonMesh.id)
		}
		if (cur.Value < 1.0f){
			UE_LOG(JsonLog, Warning, TEXT("Bone weight value %f is too low on vertex %d, mesh %s (%d)"),
				cur.Value, cur.Key, *jsonMesh.name, jsonMesh.id)
		}
	}
	*/
}


void SkeletalMeshBuilder::registerPreviewMesh(USkeleton *skel, USkeletalMesh *mesh, const JsonMesh &jsonMesh){
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
			UE_LOG(JsonLog, Log, TEXT("Skeletal mesh %d(%s) is already a part of collection"), (int)jsonMesh.id, *jsonMesh.name);
			return;
		}
	}

	auto& newVal = collection->SkeletalMeshes.AddDefaulted_GetRef();
	newVal.SkeletalMesh = mesh;

	collection->MarkPackageDirty();
	collection->PostEditChange();

	//collection->Mesh
}


/*
	Amusingly, the most useful file in figuring out how skeletal mesh configuraiton is supposed to work 
*/
void SkeletalMeshBuilder::setupSkeletalMesh(USkeletalMesh *skelMesh, const JsonMesh &jsonMesh, const JsonImporter *importer, std::function<void(TArray<FSkeletalMaterial> &meshMaterials)> materialSetup, std::function<void(const JsonSkeleton&, USkeleton*)> onNewSkeleton){
	check(skelMesh);
	check(importer);
	
	auto importModel = skelMesh->GetImportedModel();
	check(importModel->LODModels.Num() == 0);
	importModel->LODModels.Empty();

#if (ENGINE_MAJOR_VERSION >= 4) && (ENGINE_MINOR_VERSION >= 22)
	//It is not directly specified anywhere, but TIndirectArray will properly delete its elements.
	importModel->LODModels.Add(new FSkeletalMeshLODModel());
#else
	new(importModel->LODModels)FSkeletalMeshLODModel();//????
#endif
	//I suppose it does same thing as calling new and then Add()
	auto &lodModel = importModel->LODModels[0];

	auto hasNormals = jsonMesh.normals.Num() != 0;
	//auto hasColors = jsonMesh.colors.Num() != 0;
	//skelMesh->bHasVertexColors = hasColors;
	skelMesh->bHasVertexColors = (jsonMesh.colors.Num() != 0);
	auto hasTangents = jsonMesh.tangents.Num() != 0;
	//auto numTexCoords = jsonMesh.getNumTexCoords();

	skelMesh->bUseFullPrecisionUVs = true;
	skelMesh->bHasBeenSimplified = false;

	lodModel.NumTexCoords = jsonMesh.getNumTexCoords();//numTexCoords;

	if (materialSetup){
		materialSetup(skelMesh->Materials);
	}

	auto skelId = jsonMesh.defaultSkeletonId;
	if (skelId < 0){
		UE_LOG(JsonLog, Warning, TEXT("Skeleton not found on skinned mesh \"%s\"(%d)"), *jsonMesh.name, (int)jsonMesh.id);
		return;
	}

	auto jsonSkel = importer->getSkeleton(skelId);
	if (!jsonSkel){
		UE_LOG(JsonLog, Warning, TEXT("Json skeleton not found on \"%s\"(%d), skeletonid: %d"), *jsonMesh.name, (int)jsonMesh.id, skelId);
		return;
	}

	TMap<int, int> meshToSkeletonBoneMap;
	if (jsonMesh.hasBones()){
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
	}
	else{
		//Falling back to "no bones" mesh...
		const auto &defaultName = jsonMesh.defaultMeshNodeName;
		const auto defaultBoneIndex = 0;
		const auto skeletonBoneIndex = jsonSkel->findBoneIndex(defaultName);
		meshToSkeletonBoneMap.Add(defaultBoneIndex, skeletonBoneIndex);
	}

	auto &refSkeleton = skelMesh->RefSkeleton;
	setupReferenceSkeleton(refSkeleton, *jsonSkel, &jsonMesh, nullptr);//hmm.... exisitng skeleton?

	TArray<UMorphTarget*> morphTargets;

	SkeletalMeshBuildData buildData;

	buildData.startWithMesh(jsonMesh);

	TArray<FString> remapErrors;
	buildData.processPositionsAndWeights(jsonMesh, meshToSkeletonBoneMap, remapErrors);

	if (remapErrors.Num()){
		FString combinedMessage = FString::Printf(TEXT("Remap errors found while processing skeletal mesh %d(\"%s\")\n"), jsonMesh.id.toIndex(), *jsonMesh.name);
		for(const auto& cur: remapErrors){
			combinedMessage += FString::Printf(TEXT("%s\n"), *cur);
		}
	}

	buildData.processWedgeData(jsonMesh);

	buildData.buildSkeletalMesh(lodModel, refSkeleton, jsonMesh);

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

	//auto collectionSkel = skelMesh->Skeleton;

	buildData.processBlendShapes(skelMesh, jsonMesh);

	buildData.computeBoundingBox(skelMesh, jsonMesh);

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

void SkeletalMeshBuildData::processBlendShapes(USkeletalMesh *skelMesh, const JsonMesh &jsonMesh){
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
}
