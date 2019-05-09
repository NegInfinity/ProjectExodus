#include "JsonImportPrivatePCH.h"
#include "SkinMeshTest.h"
#include "MeshBuilder.h"
#include "UnrealUtilities.h"
#include "MeshUtilities.h"
#include "AssetRegistryModule.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshModel.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/Engine/Classes/Animation/Skeleton.h"
#include "Runtime/Engine/Classes/Animation/MorphTarget.h"

using namespace UnrealUtilities;

#if 0
static void fillTestSkinMeshDirect(USkeletalMesh* skelMesh){
	UE_LOG(JsonLog, Warning, TEXT("Starting skin mesh test"));
	auto importModel = skelMesh->GetImportedModel();
	check(importModel->LODModels.Num() == 0);
	importModel->LODModels.Empty();

	new(importModel->LODModels)FSkeletalMeshLODModel();//????
	auto &lodModel = importModel->LODModels[0];

	new(lodModel.Sections) FSkelMeshSection();
	auto &lodSection = lodModel.Sections[0];
	
	auto &lodInfo = skelMesh->AddLODInfo();
	lodInfo.ScreenSize = 0.3f;
	lodInfo.LODHysteresis = 0.2f;

	lodInfo.LODMaterialMap.Add(0);

	//UMaterial* defaultMat = nullptr;//UMaterial::GetDefaultMaterial(MD_Surface);
	UMaterial* defaultMat = UMaterial::GetDefaultMaterial(MD_Surface);

	skelMesh->Materials.Add(defaultMat);

	auto& refSkeleton = skelMesh->RefSkeleton;
	refSkeleton.Empty();

	skelMesh->bUseFullPrecisionUVs = true;
	skelMesh->bHasVertexColors = false;
	skelMesh->bHasBeenSimplified = false;

	FString boneName1 = TEXT("bone1");
	FString boneName2 = TEXT("bone2");
	FString boneName3 = TEXT("bone3");

	FTransform boneTransform1, boneTransform2, boneTransform3;
	boneTransform1.SetFromMatrix(FMatrix::Identity);
	boneTransform1.SetLocation(FVector(0.0f, 0.0f, 0.0f));

	boneTransform2.SetFromMatrix(FMatrix::Identity);
	boneTransform2.SetLocation(FVector(0.0f, 100.0f, 100.0f));

	boneTransform3.SetFromMatrix(FMatrix::Identity);
	///So, locations are local.
	//boneTransform3.SetLocation(FVector(0.0f, -100.0f, 100.0f));
	boneTransform3.SetLocation(FVector(0.0f, -200.0f, 0.0f));

	lodModel.RequiredBones.Add(0);
	lodModel.RequiredBones.Add(1);
	lodModel.RequiredBones.Add(2);
	lodModel.ActiveBoneIndices.Add(0);
	lodModel.ActiveBoneIndices.Add(1);
	lodModel.ActiveBoneIndices.Add(2);
	lodSection.BoneMap.Add(0);
	lodSection.BoneMap.Add(1);
	lodSection.BoneMap.Add(2);

	{
		FReferenceSkeletonModifier refSkelModifier(refSkeleton, nullptr);
		auto boneInfo1 = FMeshBoneInfo(FName(*boneName1), boneName1, INDEX_NONE);
		auto boneInfo2 = FMeshBoneInfo(FName(*boneName2), boneName2, 0);
		auto boneInfo3 = FMeshBoneInfo(FName(*boneName3), boneName3, 1);

		refSkelModifier.Add(boneInfo1, boneTransform1);
		refSkelModifier.Add(boneInfo2, boneTransform2);
		refSkelModifier.Add(boneInfo3, boneTransform3);
	}

	lodSection.BaseIndex = 0;
	lodSection.BaseVertexIndex = 0;
	lodSection.NumVertices = 4;
	lodSection.MaxBoneInfluences = 4;

	lodSection.SoftVertices.SetNumUninitialized(lodSection.NumVertices);

	TArray<FVector> meshPoints;
	TArray<FVector2D> uvs;
	meshPoints.Add(FVector(0.0f, -100.0f, 100.0f));
	uvs.Add(FVector2D(0.0f, 1.0f));
	meshPoints.Add(FVector(0.0f, 100.0f, 100.0f));
	uvs.Add(FVector2D(1.0f, 1.0f));
	meshPoints.Add(FVector(0.0f, -100.0f, -100.0f));
	uvs.Add(FVector2D(0.0f, 0.0f));
	meshPoints.Add(FVector(0.0f, 100.0f, -100.0f));
	uvs.Add(FVector2D(1.0f, 0.0f));
	FVector norm(-1.0f, 0.0f, 0.0f);
	FVector tanU(0.0f, 1.0f, 0.0f);
	FVector tanV(0.0f, 0.0f, 1.0f);

	FBox boundBox;
	boundBox.Init();

	auto &softVerts = lodSection.SoftVertices;
	for(int i = 0; i < lodSection.NumVertices; i++){
		auto &dstVert = softVerts[i];
		dstVert.Position = meshPoints[i];
		boundBox += dstVert.Position;
		dstVert.TangentX = tanU;
		dstVert.TangentY = tanV;
		dstVert.TangentZ = norm;
		dstVert.UVs[0]  = uvs[i];
	}

	for (int vertIndex = 0; vertIndex < softVerts.Num(); vertIndex++){
		for(int inflIndex = 0; inflIndex < MAX_TOTAL_INFLUENCES; inflIndex++){
			lodSection.SoftVertices[vertIndex].InfluenceBones[inflIndex] = 0;
			lodSection.SoftVertices[vertIndex].InfluenceWeights[inflIndex] = 0;
		}
		lodSection.SoftVertices[vertIndex].InfluenceWeights[0] = 255;/*Wait a minute? It is not floats? Oookay?*/
	}
	
	lodSection.SoftVertices[0].InfluenceBones[0] = 2;
	lodSection.SoftVertices[1].InfluenceBones[0] = 1;
	lodSection.SoftVertices[2].InfluenceBones[0] = 0;
	lodSection.SoftVertices[3].InfluenceBones[0] = 0;

	lodModel.NumVertices = lodSection.SoftVertices.Num();
	lodModel.NumTexCoords = 1;

	lodModel.IndexBuffer.Add(0);
	lodModel.IndexBuffer.Add(2);
	lodModel.IndexBuffer.Add(1);
	lodModel.IndexBuffer.Add(2);
	lodModel.IndexBuffer.Add(3);
	lodModel.IndexBuffer.Add(1);

	lodModel.IndexBuffer.Add(0);
	lodModel.IndexBuffer.Add(1);
	lodModel.IndexBuffer.Add(2);
	lodModel.IndexBuffer.Add(2);
	lodModel.IndexBuffer.Add(1);
	lodModel.IndexBuffer.Add(3);

	lodSection.NumTriangles = lodModel.IndexBuffer.Num()/3;

	FBoxSphereBounds bounds(boundBox);
	//bounds = bounds.ExpandBy
	skelMesh->SetImportedBounds(bounds);

	skelMesh->PostEditChange();

	auto skeleton = NewObject<USkeleton>(skelMesh->GetOuter());
	skeleton->SetFlags(RF_Public|RF_Standalone);
	skeleton->MergeAllBonesToBoneTree(skelMesh);
	skeleton->MarkPackageDirty();
	//FassetDatabase

	skelMesh->Skeleton = skeleton;

	skelMesh->MarkPackageDirty();
	skelMesh->PostLoad();
}
#endif

static void fillTestSkinMeshWithMeshTools(USkeletalMesh* skelMesh){
	FlushRenderingCommands();

	UE_LOG(JsonLog, Warning, TEXT("Starting skin mesh test"));
	auto importModel = skelMesh->GetImportedModel();
	check(importModel->LODModels.Num() == 0);
	importModel->LODModels.Empty();

	new(importModel->LODModels)FSkeletalMeshLODModel();//????
	auto &lodModel = importModel->LODModels[0];
	auto numTexCoords = 1;
	lodModel.NumTexCoords = numTexCoords;

	UMaterial* defaultMat = UMaterial::GetDefaultMaterial(MD_Surface);
	skelMesh->Materials.Add(defaultMat);

	auto& refSkeleton = skelMesh->RefSkeleton;
	refSkeleton.Empty();

	skelMesh->bUseFullPrecisionUVs = true;
	skelMesh->bHasVertexColors = false;
	skelMesh->bHasBeenSimplified = false;

	FString boneName1 = TEXT("bone1");
	FString boneName2 = TEXT("bone2");
	FString boneName3 = TEXT("bone3");

	FTransform boneTransform1, boneTransform2, boneTransform3;
	boneTransform1.SetFromMatrix(FMatrix::Identity);
	boneTransform1.SetLocation(FVector(0.0f, 0.0f, 0.0f));
	boneTransform2.SetFromMatrix(FMatrix::Identity);
	boneTransform2.SetLocation(FVector(0.0f, 100.0f, 100.0f));
	boneTransform3.SetFromMatrix(FMatrix::Identity);
	///So, locations are local.
	//boneTransform3.SetLocation(FVector(0.0f, -100.0f, 100.0f));
	boneTransform3.SetLocation(FVector(0.0f, -200.0f, 0.0f));

	{
		FReferenceSkeletonModifier refSkelModifier(refSkeleton, nullptr);
		auto boneInfo1 = FMeshBoneInfo(FName(*boneName1), boneName1, INDEX_NONE);
		auto boneInfo2 = FMeshBoneInfo(FName(*boneName2), boneName2, 0);
		auto boneInfo3 = FMeshBoneInfo(FName(*boneName3), boneName3, 1);

		refSkelModifier.Add(boneInfo1, boneTransform1);
		refSkelModifier.Add(boneInfo2, boneTransform2);
		refSkelModifier.Add(boneInfo3, boneTransform3);
	}

	/*
		Alright....
		meshPoints - base underlying geometry vertices.
		meshInfluences - apparently per-vertex bone weights, except that they refer to mesh points.
		meshWedges - misc data such as uvs and vertex color that is attached to poitns.... except that instead of position it refers to mesh point index.
		meshFaces -- actual faces, that store indexes to meshWedges.
		pointToOriginalMap --> apparently maps meshPoints to some "original", whatever it is suppsoed to be. Sources mention blendshapes.

		So, geometry is defined by meshPoints, 
		meshWedges are advanced vertex format that includes INDEX to basic geometry points,
		meshFaces - refer to wedges by index and I think hold some extra data.

		Influences is just a linear array with index + weight pairs. 

		And pointToOriginal is 1-to-1 map between poitns and something else (external model?)

		That... should be about it?

		Now, we CAN reinvent the wheel and fill renderable data ourselves, BUT this wouldn't compute normals, and will be a bit error prone for future releases.
		Then again in a future release half of this code might disappear altogether.
	*/
	TArray<SkeletalMeshImportData::FVertInfluence> meshInfluences;
	TArray<SkeletalMeshImportData::FMeshWedge> meshWedges;
	TArray<SkeletalMeshImportData::FMeshFace> meshFaces;
	TArray<FVector> meshPoints;
	TArray<int32> pointToOriginalMap;
	TArray<FText> buildWarnMessages;
	TArray<FName> buildWarnNames;

	TArray<FMorphTargetDelta> originalDeltas;

	TArray<FVector2D> uvs;
	meshPoints.Add(FVector(0.0f, -100.0f, 100.0f));
	uvs.Add(FVector2D(0.0f, 1.0f));
	originalDeltas.Add(FMorphTargetDelta{FVector(0.0f, -100.0f, 100.0f), FVector(0.0f, 0.0f, 0.0f), 0});

	meshPoints.Add(FVector(0.0f, 100.0f, 100.0f));
	uvs.Add(FVector2D(1.0f, 1.0f));
	originalDeltas.Add(FMorphTargetDelta{FVector(0.0f, 100.0f, 100.0f), FVector(0.0f, 0.0f, 0.0f), 1});

	meshPoints.Add(FVector(0.0f, -100.0f, -100.0f));
	uvs.Add(FVector2D(0.0f, 0.0f));
	originalDeltas.Add(FMorphTargetDelta{FVector(0.0f, -100.0f, -100.0f), FVector(0.0f, 0.0f, 0.0f), 2});

	meshPoints.Add(FVector(0.0f, 100.0f, -100.0f));
	uvs.Add(FVector2D(1.0f, 0.0f));
	originalDeltas.Add(FMorphTargetDelta{FVector(0.0f, 100.0f, -100.0f), FVector(0.0f, 0.0f, 0.0f), 3});

	FVector norm(-1.0f, 0.0f, 0.0f);
	FVector tanU(0.0f, 1.0f, 0.0f);
	FVector tanV(0.0f, 0.0f, 1.0f);
	IntArray indexes = {
		0, 2, 1, 2, 3, 1, 0, 1, 2, 2, 1, 3
	};

	for(int i = 0; i < meshPoints.Num(); i++){
		pointToOriginalMap.Add(i);
	}

	auto* infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 2;
	infl->VertIndex = 0;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 1;
	infl->VertIndex = 1;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 0;
	infl->VertIndex = 2;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 0;
	infl->VertIndex = 3;
	infl->Weight = 1.0f;
	/*
	lodSection.SoftVertices[0].InfluenceBones[0] = 2;
	lodSection.SoftVertices[1].InfluenceBones[0] = 1;
	lodSection.SoftVertices[2].InfluenceBones[0] = 0;
	lodSection.SoftVertices[3].InfluenceBones[0] = 0;
	*/

	for(int indexOffset = 0; (indexOffset + 2) < indexes.Num(); indexOffset += 3){
		auto &dstFace = meshFaces.AddDefaulted_GetRef();
		dstFace.SmoothingGroups = 0;
		dstFace.MeshMaterialIndex = 0;
		for(int faceVert = 0; faceVert < 3; faceVert++){
			dstFace.TangentX[faceVert] = tanU;
			dstFace.TangentY[faceVert] = tanV;
			dstFace.TangentZ[faceVert] = norm;

			auto wedgeIndex = indexOffset + faceVert;
			auto srcVertIdx = indexes[wedgeIndex];
			dstFace.iWedge[faceVert] = wedgeIndex;

			auto &dstWedge = meshWedges.AddDefaulted_GetRef();
			dstWedge.iVertex = srcVertIdx;
			dstWedge.UVs[0] = uvs[srcVertIdx];
			dstWedge.Color = FColor::White;
		}
		//auto& dstFace = 
	}

	IMeshUtilities::MeshBuildOptions buildOptions;
	buildOptions.bComputeNormals = false;
	buildOptions.bComputeTangents = false;
	buildOptions.bUseMikkTSpace = false;
	IMeshUtilities& meshUtils = FModuleManager::Get().LoadModuleChecked<IMeshUtilities>("MeshUtilities");
	/*
	Looks like only REFERENCE skeleton is being used in BuildSkeletalMesh.
	Woudl be nice to have it documented....
	*/
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
	skeleton->SetFlags(RF_Public|RF_Standalone);
	skeleton->MergeAllBonesToBoneTree(skelMesh);
	skelMesh->Skeleton = skeleton;
	FAssetRegistryModule::AssetCreated(skeleton);
	skeleton->MarkPackageDirty();

	/*
	auto morphTarget = NewObject<UMorphTarget>(skelMesh->GetOuter());
	FAssetRegistryModule::AssetCreated(morphTarget);
	*/
	/*
	TArray<FMorphTargetDelta> deltas;
	auto morphLodModel = skelMesh->GetImportedModel()->LODModels[0];
	*/

	for(int shapeIndex = 0; shapeIndex < 5; shapeIndex++){
		auto morphTarget = NewObject<UMorphTarget>(skelMesh->GetOuter());
		FAssetRegistryModule::AssetCreated(morphTarget);

		TArray<FMorphTargetDelta> deltas;
		const auto morphModel = skelMesh->GetImportedModel();
		const auto &morphLodModel = morphModel->LODModels[0];

		bool all = shapeIndex == 4;

		//morphLodModel.
		//UE_LOG(JsonLog, Log, TEXT(""), morephModel->);
		UE_LOG(JsonLog, Log, TEXT("morphLodModel.MeshToImportVertexMap.Num() -> %d"), morphLodModel.MeshToImportVertexMap.Num());
		for(int vertIndex = 0; vertIndex < morphLodModel.MeshToImportVertexMap.Num(); vertIndex++){
			auto deltaIndex = morphLodModel.MeshToImportVertexMap[vertIndex];
			UE_LOG(JsonLog, Log, TEXT("vertIndex %d => %d"), deltaIndex);
			auto origIndex = originalDeltas[deltaIndex];
			auto origDelta = originalDeltas[deltaIndex];
			if (!all && (shapeIndex != deltaIndex))
				continue;
			auto newDelta = origDelta;
			newDelta.SourceIdx = vertIndex;
			//deltas.Add(FMorphTargetDelta{origDelta.PositionDelta, origDelta.TangentZDelta, 
			//deltas.Add(origDelta);
			deltas.Add(newDelta);
		}
	#if 0
		UE_LOG(JsonLog, Log, TEXT("morphLodModel.MeshToImportVertexMap.Num() -> %d"), morphLodModel.MeshToImportVertexMap.Num());
		for(int vertIndex = 0; vertIndex < morphLodModel.IndexBuffer.Num(); vertIndex++){
		//for(int vertIndex = 0; vertIndex < morphLodModel.MeshToImportVertexMap.Num(); vertIndex++){
			auto curIndex = morphLodModel.IndexBuffer[vertIndex];
			//auto deltaIndex = morphLodModel.MeshToImportVertexMap[vertIndex];
			auto deltaIndex = morphLodModel.MeshToImportVertexMap[curIndex];
			UE_LOG(JsonLog, Log, TEXT("vertIndex %d => %d"), deltaIndex);
			auto origIndex = originalDeltas[deltaIndex];
		//for(int deltaIndex = 0; deltaIndex < originalDeltas.Num(); deltaIndex++){
			auto origDelta = originalDeltas[deltaIndex];
			if (!all && (shapeIndex != deltaIndex))
				continue;
			deltas.Add(origDelta);
		}
	#endif
		morphTarget->PopulateDeltas(deltas, 0, morphLodModel.Sections);
		morphTarget->MarkPackageDirty();
		auto registrationResult = skelMesh->RegisterMorphTarget(morphTarget);
	}

	/*
	for(int deltaIndex = 0; deltaIndex < originalDeltas.Num(); deltaIndex++){
		auto origDelta = originalDeltas[i];
		deltas.Add(origDelta);
	}
	*/

	/*
	auto &dstDelta = deltas.AddDefaulted_GetRef();
	dstDelta.PositionDelta = FVector(100.0f, 0.0f, 0.0f);
	dstDelta.TangentZDelta = FVector(0.0f, 0.0f, 0.0f);
	
	for(int i = 0; i < meshPoints.Num(); i++){
		auto &dstDelta = deltas.AddDefaulted_GetRef();
		dstDelta.PositionDelta = FVector(100.0f, 0.0f, 0.0f);
		dstDelta.TangentZDelta = FVector(0.0f, 0.0f, 0.0f);
		dstDelta.SourceIdx = 0;
	}
	*/

	/*
	morphTarget->PopulateDeltas(deltas, 0, morphLodModel.Sections);
	morphTarget->MarkPackageDirty();
	auto registrationResult = skelMesh->RegisterMorphTarget(morphTarget);
	*/

	skelMesh->PostEditChange();

	skelMesh->MarkPackageDirty();
	skelMesh->PostLoad();
}

void SkinMeshTest::run(){
	FString meshName = TEXT("SkinMeshDebug");
	FString meshPath = TEXT("SkinMeshDebug");
	//auto fullPath = meshName;//FString::Printf(TEXT("%s/%s"), *meshPath, *meshName);

	auto skelMesh = createAssetObject<USkeletalMesh>(meshName, nullptr, nullptr, 
		[&](auto arg){
			fillTestSkinMeshWithMeshTools(arg);
			//fillTestSkinMeshDirect(arg);
		}, RF_Standalone|RF_Public);
}
