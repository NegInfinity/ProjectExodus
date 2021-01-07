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

#include "Runtime/Launch/Resources/Version.h"

#include "IMeshBuilderModule.h"

using namespace UnrealUtilities;

#ifdef EXODUS_UE_VER_4_24_GE

struct TestSkelMeshFiller{
	USkeletalMesh* skelMesh = {};

	void fill(USkeletalMesh* mesh_);
protected:
	TArray<FMorphTargetDelta> originalDeltas;

	void handleRefSkeleton();
	void preEdit();
	void postEdit();

	void createSkeleton();
	void addDefaultMaterial();
	void buildImportData();
	void addLods();

	void createBones(FSkeletalMeshImportData& importData);
	void buildGeometry(FSkeletalMeshImportData& importData);
	void buildSkeletalMesh(FSkeletalMeshImportData& importData);

	void buildMorphs(FSkeletalMeshImportData& importData);
};

void TestSkelMeshFiller::fill(USkeletalMesh* mesh_){
	skelMesh = mesh_;

	preEdit();
	handleRefSkeleton();
	buildImportData();
	postEdit();
}

void TestSkelMeshFiller::preEdit(){
	FlushRenderingCommands();
	skelMesh->PreEditChange(NULL);
	skelMesh->InvalidateDeriveDataCacheGUID();
}

void TestSkelMeshFiller::handleRefSkeleton(){
	auto& refSkeleton = skelMesh->RefSkeleton;
	refSkeleton.Empty();
}

void TestSkelMeshFiller::postEdit(){
	skelMesh->PostEditChange();
	skelMesh->MarkPackageDirty();
	skelMesh->PostLoad();
}

void TestSkelMeshFiller::addDefaultMaterial(){
	UMaterial* defaultMat = UMaterial::GetDefaultMaterial(MD_Surface);
	skelMesh->Materials.Add(defaultMat);
}

void TestSkelMeshFiller::createSkeleton(){
	auto skeleton = NewObject<USkeleton>(skelMesh->GetOuter());
	skeleton->SetFlags(RF_Public|RF_Standalone);
	skeleton->MergeAllBonesToBoneTree(skelMesh);
	skelMesh->Skeleton = skeleton;// <-- skeleton setting
	FAssetRegistryModule::AssetCreated(skeleton);
	skeleton->MarkPackageDirty();
}

static void fillTestSkinMeshWithMeshTools(USkeletalMesh* skelMesh){
	TestSkelMeshFiller meshFiller;
	meshFiller.fill(skelMesh);
}

void TestSkelMeshFiller::buildImportData(){
	/*
		At some point after 4.21 unreal devs combined all the mesh data into single structure. FSkeletalMeshImportData
	*/
	FSkeletalMeshImportData importData;

	createBones(importData);
	//addDefaultMaterial();
	//buildGeometry(importData);
	//buildMorphs();

	buildSkeletalMesh(importData);
	createSkeleton();
}

void TestSkelMeshFiller::buildSkeletalMesh(FSkeletalMeshImportData& importData){
	FSkeletalMeshBuildSettings buildSettings;
	buildSettings.bRecomputeNormals = false;
	buildSettings.bRecomputeTangents = false;

	int32 lodIndex = 0;
	auto newLodInfo = skelMesh->AddLODInfo();
	newLodInfo.ReductionSettings.NumOfTrianglesPercentage = 1.0f;
	newLodInfo.ReductionSettings.NumOfVertPercentage = 1.0f;
	newLodInfo.ReductionSettings.MaxDeviationPercentage = 0.0f;
	newLodInfo.LODHysteresis = 0.02f;

	auto lod = skelMesh->GetLODInfo(lodIndex);
	check(lod != nullptr);

	auto meshBuildModule = FModuleManager::Get().LoadModuleChecked<IMeshBuilderModule>("MeshBuilder");
	auto success = meshBuildModule.BuildSkeletalMesh(skelMesh, lodIndex, false);
}

void TestSkelMeshFiller::addLods(){
	auto importModel = skelMesh->GetImportedModel();
	check(importModel->LODModels.Num() == 0);
	importModel->LODModels.Empty();

	//It is not directly specified anywhere, but TIndirectArray will properly delete its elements.
	importModel->LODModels.Add(new FSkeletalMeshLODModel());

	auto& lodModel = importModel->LODModels[0];
	auto numTexCoords = 1;
	lodModel.NumTexCoords = numTexCoords;
}

void TestSkelMeshFiller::createBones(FSkeletalMeshImportData& importData){
	FString boneName1 = TEXT("bone1");
	FString boneName2 = TEXT("bone2");
	FString boneName3 = TEXT("bone3");

	auto& importDataBones = importData.RefBonesBinary;

	SkeletalMeshImportData::FBone rawBone1, rawBone2, rawBone3;

	FTransform boneTransform1, boneTransform2, boneTransform3;
	boneTransform1.SetFromMatrix(FMatrix::Identity);
	boneTransform1.SetLocation(FVector(0.0f, 0.0f, 0.0f));
	boneTransform2.SetFromMatrix(FMatrix::Identity);
	boneTransform2.SetLocation(FVector(0.0f, 100.0f, 100.0f));
	boneTransform3.SetFromMatrix(FMatrix::Identity);

	///So, locations are local.
	//boneTransform3.SetLocation(FVector(0.0f, -100.0f, 100.0f));
	boneTransform3.SetLocation(FVector(0.0f, -200.0f, 0.0f));

	rawBone1.Name = boneName1;
	rawBone1.BonePos.Transform = boneTransform1;
	rawBone1.ParentIndex = INDEX_NONE;

	rawBone2.Name = boneName2;
	rawBone2.BonePos.Transform = boneTransform2;
	rawBone2.ParentIndex = 0;

	rawBone3.Name = boneName3;
	rawBone3.BonePos.Transform = boneTransform3;
	rawBone3.ParentIndex = 1;

	importDataBones.Add(rawBone1);
	importDataBones.Add(rawBone2);
	importDataBones.Add(rawBone3);

	{
		auto& refSkeleton = skelMesh->RefSkeleton;
		FReferenceSkeletonModifier refSkelModifier(refSkeleton, nullptr);
		auto boneInfo1 = FMeshBoneInfo(FName(*rawBone1.Name), rawBone1.Name, rawBone1.ParentIndex);//FName(*boneName1), boneName1, INDEX_NONE);
		auto boneInfo2 = FMeshBoneInfo(FName(*boneName2), boneName2, 0);
		auto boneInfo3 = FMeshBoneInfo(FName(*boneName3), boneName3, 1);

		refSkelModifier.Add(boneInfo1, rawBone1.BonePos.Transform);//boneTransform1);
		refSkelModifier.Add(boneInfo2, rawBone2.BonePos.Transform);//boneTransform2);
		refSkelModifier.Add(boneInfo3, rawBone3.BonePos.Transform);//boneTransform3);
	}
}

void TestSkelMeshFiller::buildGeometry(FSkeletalMeshImportData& importData){
	return;

	auto& meshInfluences = importData.Influences;
	auto& meshWedges = importData.Wedges;
	auto& meshFaces = importData.Faces;
	auto& meshPoints = importData.Points;
	auto& pointToOriginalMap = importData.PointToRawMap;

	TArray<FText> buildWarnMessages;
	TArray<FName> buildWarnNames;

	originalDeltas.Empty();

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

	for (int i = 0; i < meshPoints.Num(); i++){
		pointToOriginalMap.Add(i);
	}

	auto* infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 2;
	infl->VertexIndex = 0;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 1;
	infl->VertexIndex = 1;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 0;
	infl->VertexIndex = 2;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 0;
	infl->VertexIndex = 3;
	infl->Weight = 1.0f;

	for (int indexOffset = 0; (indexOffset + 2) < indexes.Num(); indexOffset += 3){
		auto& dstFace = meshFaces.AddDefaulted_GetRef();
		dstFace.SmoothingGroups = 0;
		dstFace.MatIndex = 0;
		for (int faceVert = 0; faceVert < 3; faceVert++){
			dstFace.TangentX[faceVert] = tanU;
			dstFace.TangentY[faceVert] = tanV;
			dstFace.TangentZ[faceVert] = norm;

			auto wedgeIndex = indexOffset + faceVert;
			auto srcVertIdx = indexes[wedgeIndex];
			dstFace.WedgeIndex[faceVert] = wedgeIndex;

			auto& dstWedge = meshWedges.AddDefaulted_GetRef();
			dstWedge.VertexIndex = srcVertIdx;
			dstWedge.UVs[0] = uvs[srcVertIdx];
			dstWedge.Color = FColor::White;
		}
	}
}

void TestSkelMeshFiller::buildMorphs(FSkeletalMeshImportData& importData){
	//	TArray<FMorphTargetDelta> originalDeltas; <-- this is no longer stored in skeletal mesh import model, and thats' why morphs are unitialiezzed.
	return;

	for (int shapeIndex = 0; shapeIndex < 5; shapeIndex++){
		auto morphTarget = NewObject<UMorphTarget>(skelMesh->GetOuter());
		FAssetRegistryModule::AssetCreated(morphTarget);

		TArray<FMorphTargetDelta> deltas;
		const auto morphModel = skelMesh->GetImportedModel();
		const auto& morphLodModel = morphModel->LODModels[0];

		bool all = shapeIndex == 4;

		//morphLodModel.
		//UE_LOG(JsonLog, Log, TEXT(""), morephModel->);
		UE_LOG(JsonLog, Log, TEXT("morphLodModel.MeshToImportVertexMap.Num() -> %d"), morphLodModel.MeshToImportVertexMap.Num());
		for (int vertIndex = 0; vertIndex < morphLodModel.MeshToImportVertexMap.Num(); vertIndex++){
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
		for (int vertIndex = 0; vertIndex < morphLodModel.IndexBuffer.Num(); vertIndex++){
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
}

#if 0
static void fillTestSkinMeshWithMeshTools(USkeletalMesh* skelMesh){
	FlushRenderingCommands();

	skelMesh->PreEditChange(NULL);
	skelMesh->InvalidateDeriveDataCacheGUID();

	UE_LOG(JsonLog, Warning, TEXT("Starting skin mesh test"));
	auto importModel = skelMesh->GetImportedModel();
	check(importModel->LODModels.Num() == 0);
	importModel->LODModels.Empty();

//#if (ENGINE_MAJOR_VERSION >= 4) && (ENGINE_MINOR_VERSION >= 22)
#ifdef EXODUS_UE_VER_4_22_GE
	//It is not directly specified anywhere, but TIndirectArray will properly delete its elements.
	importModel->LODModels.Add(new FSkeletalMeshLODModel());
#else
	new(importModel->LODModels)FSkeletalMeshLODModel();//????
#endif
	auto &lodModel = importModel->LODModels[0];
	auto numTexCoords = 1;
	lodModel.NumTexCoords = numTexCoords;

	UMaterial* defaultMat = UMaterial::GetDefaultMaterial(MD_Surface);
	skelMesh->Materials.Add(defaultMat);

	auto& refSkeleton = skelMesh->RefSkeleton;
	refSkeleton.Empty();

//#if ! ((ENGINE_MAJOR_VERSION >= 4) && (ENGINE_MINOR_VERSION >= 24))
#ifndef EXODUS_UE_VER_4_24_GE
	skelMesh->bUseFullPrecisionUVs = true;
#endif
	skelMesh->bHasVertexColors = false;
	skelMesh->bHasBeenSimplified = false;

	FString boneName1 = TEXT("bone1");
	FString boneName2 = TEXT("bone2");
	FString boneName3 = TEXT("bone3");

	/*
		At some point after 4.21 unreal devs combined all the mesh data into single structure. FSkeletalMeshImportData
	*/
	FSkeletalMeshImportData importData;
	auto& importDataBones = importData.RefBonesBinary;

	SkeletalMeshImportData::FBone rawBone1, rawBone2, rawBone3;

	FTransform boneTransform1, boneTransform2, boneTransform3;

	boneTransform1.SetFromMatrix(FMatrix::Identity);
	boneTransform1.SetLocation(FVector(0.0f, 0.0f, 0.0f));
	boneTransform2.SetFromMatrix(FMatrix::Identity);
	boneTransform2.SetLocation(FVector(0.0f, 100.0f, 100.0f));
	boneTransform3.SetFromMatrix(FMatrix::Identity);
	///So, locations are local.
	//boneTransform3.SetLocation(FVector(0.0f, -100.0f, 100.0f));
	boneTransform3.SetLocation(FVector(0.0f, -200.0f, 0.0f));

	rawBone1.Name = boneName1;
	rawBone1.BonePos.Transform = boneTransform1;
	rawBone1.ParentIndex = INDEX_NONE;

	rawBone2.Name = boneName2;
	rawBone2.BonePos.Transform = boneTransform2;
	rawBone2.ParentIndex = 0;

	rawBone3.Name = boneName3;
	rawBone3.BonePos.Transform = boneTransform3;
	rawBone3.ParentIndex = 1;

	importDataBones.Add(rawBone1);
	importDataBones.Add(rawBone2);
	importDataBones.Add(rawBone3);

	{
		FReferenceSkeletonModifier refSkelModifier(refSkeleton, nullptr);
		auto boneInfo1 = FMeshBoneInfo(FName(*rawBone1.Name), rawBone1.Name, rawBone1.ParentIndex);//FName(*boneName1), boneName1, INDEX_NONE);
		auto boneInfo2 = FMeshBoneInfo(FName(*boneName2), boneName2, 0);
		auto boneInfo3 = FMeshBoneInfo(FName(*boneName3), boneName3, 1);

		refSkelModifier.Add(boneInfo1, rawBone1.BonePos.Transform);//boneTransform1);
		refSkelModifier.Add(boneInfo2, rawBone2.BonePos.Transform);//boneTransform2);
		refSkelModifier.Add(boneInfo3, rawBone3.BonePos.Transform);//boneTransform3);
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

#if 1 //geometry

	//TArray<SkeletalMeshImportData::FVertInfluence> meshInfluences;
	auto& meshInfluences = importData.Influences;

	//TArray<SkeletalMeshImportData::FMeshWedge> meshWedges;
	auto& meshWedges = importData.Wedges;

	//TArray<SkeletalMeshImportData::FMeshFace> meshFaces;
	auto& meshFaces = importData.Faces;

	//TArray<FVector> meshPoints;
	auto& meshPoints = importData.Points;

	//TArray<int32> pointToOriginalMap;
	auto& pointToOriginalMap = importData.PointToRawMap;

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
	//infl->VertIndex = 0;
	infl->VertexIndex = 0;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 1;
	//infl->VertIndex = 1;
	infl->VertexIndex = 1;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 0;
	//infl->VertIndex = 2;
	infl->VertexIndex = 2;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 0;
	//infl->VertIndex = 3;
	infl->VertexIndex = 3;
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
		//dstFace.MeshMaterialIndex = 0;
		dstFace.MatIndex = 0;
		for(int faceVert = 0; faceVert < 3; faceVert++){
			dstFace.TangentX[faceVert] = tanU;
			dstFace.TangentY[faceVert] = tanV;
			dstFace.TangentZ[faceVert] = norm;

			auto wedgeIndex = indexOffset + faceVert;
			auto srcVertIdx = indexes[wedgeIndex];
			//dstFace.iWedge[faceVert] = wedgeIndex;
			dstFace.WedgeIndex[faceVert] = wedgeIndex;

			auto &dstWedge = meshWedges.AddDefaulted_GetRef();
			//dstWedge.iVertex = srcVertIdx;
			dstWedge.VertexIndex = srcVertIdx;
			dstWedge.UVs[0] = uvs[srcVertIdx];
			dstWedge.Color = FColor::White;
		}
		//auto& dstFace = 
	}
#endif

#if 1
	//FSkeletalMeshImportData meshImportData;
	//meshImportData.CopyLODImportData(meshPoints, meshWedges, meshFaces, meshInfluences, pointToOriginalMap);

	FSkeletalMeshBuildSettings buildSettings;
	buildSettings.bRecomputeNormals = false;
	buildSettings.bRecomputeTangents = false;

	int32 lodIndex = 0;
	auto newLodInfo = skelMesh->AddLODInfo();
	newLodInfo.ReductionSettings.NumOfTrianglesPercentage = 1.0f;
	newLodInfo.ReductionSettings.NumOfVertPercentage = 1.0f;
	newLodInfo.ReductionSettings.MaxDeviationPercentage = 0.0f;
	newLodInfo.LODHysteresis = 0.02f;

	auto lod = skelMesh->GetLODInfo(lodIndex);
	check(lod != nullptr);

	auto meshBuildModule = FModuleManager::Get().LoadModuleChecked<IMeshBuilderModule>("MeshBuilder");
	auto success = meshBuildModule.BuildSkeletalMesh(skelMesh, lodIndex, false);
#endif

#if 0
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
#endif

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

#if 0 //shapes
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
#endif

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
#endif

#endif //EXODUS_UE_VER_4_24_GE

void SkinMeshTest::run(){
#ifdef EXODUS_UE_VER_4_24_GE
	FString meshName = TEXT("SkinMeshDebug");
	FString meshPath = TEXT("SkinMeshDebug");
	//auto fullPath = meshName;//FString::Printf(TEXT("%s/%s"), *meshPath, *meshName);

	auto skelMesh = createAssetObject<USkeletalMesh>(meshName, nullptr, nullptr, 
		[&](auto arg){
			fillTestSkinMeshWithMeshTools(arg);
			//fillTestSkinMeshDirect(arg);
		}, RF_Standalone|RF_Public);
#endif //EXODUS_UE_VER_4_24_GE
}
