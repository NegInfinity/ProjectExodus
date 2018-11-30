#include "JsonImportPrivatePCH.h"
#include "SkinMeshTest.h"
#include "MeshBuilder.h"
#include "UnrealUtilities.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshModel.h"
#include "Runtime/Engine/Classes/Materials/Material.h"

using namespace UnrealUtilities;

static void fillTestSkinMesh(USkeletalMesh* skelMesh){
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

	UMaterial* defaultMat = nullptr;//UMaterial::GetDefaultMaterial(MD_Surface);

	skelMesh->Materials.Add(defaultMat);

	auto& refSkeleton = skelMesh->RefSkeleton;
	refSkeleton.Empty();

	skelMesh->bUseFullPrecisionUVs = true;
	skelMesh->bHasVertexColors = false;
	skelMesh->bHasBeenSimplified = false;

	FString boneName1 = TEXT("bone1");
	FString boneName2 = TEXT("bone2");

	FTransform boneTransform1, boneTransform2;
	boneTransform1.SetFromMatrix(FMatrix::Identity);
	boneTransform1.SetLocation(FVector(0.0f, 0.0f, 0.0f));

	boneTransform2.SetFromMatrix(FMatrix::Identity);
	boneTransform2.SetLocation(FVector(0.0f, 0.0f, 200.0f));


	lodModel.RequiredBones.Add(0);
	lodModel.RequiredBones.Add(1);
	lodModel.ActiveBoneIndices.Add(0);
	lodModel.ActiveBoneIndices.Add(1);
	lodSection.BoneMap.Add(0);
	lodSection.BoneMap.Add(1);

	{
		FReferenceSkeletonModifier refSkelModifier(refSkeleton, nullptr);
		auto boneInfo1 = FMeshBoneInfo(FName(*boneName1), boneName1, INDEX_NONE);
		auto boneInfo2 = FMeshBoneInfo(FName(*boneName2), boneName2, 0);

		refSkelModifier.Add(boneInfo1, boneTransform1);
		refSkelModifier.Add(boneInfo2, boneTransform2);
	}

	lodSection.BaseIndex = 0;
	lodSection.BaseVertexIndex = 0;
	lodSection.NumVertices = 4;
	lodSection.MaxBoneInfluences = 4;

	lodSection.SoftVertices.SetNumUninitialized(lodSection.NumVertices);

	TArray<FVector> meshPoints;
	TArray<FVector2D> uvs;
	meshPoints.Add(FVector(0.0f, -100.0f, 200.0f));
	uvs.Add(FVector2D(0.0f, 1.0f));
	meshPoints.Add(FVector(0.0f, 100.0f, 200.0f));
	uvs.Add(FVector2D(1.0f, 1.0f));
	meshPoints.Add(FVector(0.0f, -100.0f, -200.0f));
	uvs.Add(FVector2D(0.0f, 0.0f));
	meshPoints.Add(FVector(0.0f, 100.0f, -200.0f));
	uvs.Add(FVector2D(1.0f, 0.0f));
	FVector norm(-1.0f, 0.0f, 0.0f);
	FVector tanU(0.0f, 1.0f, 0.0f);
	FVector tanV(0.0f, 0.0f, 1.0f);

	auto &softVerts = lodSection.SoftVertices;
	for(int i = 0; i < lodSection.NumVertices; i++){
		auto &dstVert = softVerts[i];
		dstVert.Position = meshPoints[i];
		dstVert.TangentX = tanU;
		dstVert.TangentY = tanV;
		dstVert.TangentZ = norm;
		dstVert.UVs[0]  = uvs[i];
	}

	for(int inflIndex = 0; inflIndex < MAX_TOTAL_INFLUENCES; inflIndex++){
		for (int i = 0; i < softVerts.Num(); i++){
			lodSection.SoftVertices[i].InfluenceBones[inflIndex] = 0;
			lodSection.SoftVertices[i].InfluenceWeights[inflIndex] = 0.0f;
		}
	}
	
	lodSection.SoftVertices[0].InfluenceBones[0] = 1;
	lodSection.SoftVertices[0].InfluenceWeights[0] = 1.0f;
	lodSection.SoftVertices[1].InfluenceBones[0] = 1;
	lodSection.SoftVertices[1].InfluenceWeights[0] = 1.0f;
	lodSection.SoftVertices[2].InfluenceBones[0] = 0;
	lodSection.SoftVertices[2].InfluenceWeights[0] = 1.0f;
	lodSection.SoftVertices[3].InfluenceBones[0] = 0;
	lodSection.SoftVertices[3].InfluenceWeights[0] = 1.0f;


	lodModel.NumVertices = lodSection.SoftVertices.Num();
	lodModel.NumTexCoords = 1;

	lodModel.IndexBuffer.Add(0);
	lodModel.IndexBuffer.Add(2);
	lodModel.IndexBuffer.Add(1);
	lodModel.IndexBuffer.Add(0);
	lodModel.IndexBuffer.Add(3);
	lodModel.IndexBuffer.Add(2);

	lodModel.IndexBuffer.Add(0);
	lodModel.IndexBuffer.Add(1);
	lodModel.IndexBuffer.Add(2);
	lodModel.IndexBuffer.Add(0);
	lodModel.IndexBuffer.Add(2);
	lodModel.IndexBuffer.Add(3);

	lodSection.NumTriangles = lodModel.IndexBuffer.Num()/3;

	skelMesh->PostEditChange();

	skelMesh->Skeleton = NewObject<USkeleton>();
	skelMesh->Skeleton->MergeAllBonesToBoneTree(skelMesh);
	skelMesh->PostLoad();
}

#if 0
static void fillTestSkinMesh(USkeletalMesh* skelMesh){
	UE_LOG(JsonLog, Warning, TEXT("Starting skin mesh test"));
	auto importModel = skelMesh->GetImportedModel();
	check(importModel->LODModels.Num() == 0);
	importModel->LODModels.Empty();

	new(importModel->LODModels)FSkeletalMeshLODModel();//????
	auto &lodModel = importModel->LODModels[0];
	auto &lodInfo = skelMesh->AddLODInfo();

	auto defaultMat = UMaterial::GetDefaultMaterial(MD_Surface);

	skelMesh->Materials.Add(defaultMat);

	for (int i = 0; i < skelMesh->Materials.Num(); i++){
		lodInfo.LODMaterialMap.Add(i);
	}

	auto& refSkeleton = skelMesh->RefSkeleton;
	refSkeleton.Empty();

	FString boneName1 = TEXT("bone1");
	FString boneName2 = TEXT("bone2");

	FTransform boneTransform1, boneTransform2;
	boneTransform1.SetLocation(FVector(0.0f, 0.0f, 0.0f));
	boneTransform2.SetLocation(FVector(0.0f, 0.0f, 100.0f));

	lodModel.RequiredBones.Add(0);
	lodModel.RequiredBones.Add(1);
	lodModel.ActiveBoneIndices.Add(0);
	lodModel.ActiveBoneIndices.Add(1);

	{
		FReferenceSkeletonModifier refSkelModifier(refSkeleton, nullptr);
		auto boneInfo1 = FMeshBoneInfo(FName(*boneName1), boneName1, INDEX_NONE);
		auto boneInfo2 = FMeshBoneInfo(FName(*boneName2), boneName2, 0);

		refSkelModifier.Add(boneInfo1, boneTransform1);
		refSkelModifier.Add(boneInfo2, boneTransform2);
	}

	auto numTexCoords = 1;

	TArray<SkeletalMeshImportData::FVertInfluence> meshInfluences;
	TArray<SkeletalMeshImportData::FMeshWedge> meshWedges;
	TArray<SkeletalMeshImportData::FMeshFace> meshFaces;
	TArray<FVector> meshPoints;
	TArray<int32> pointToOriginalMap;
	TArray<FText> buildWarnMessages;
	TArray<FName> buildWarnNames;

	TArray<FVector2D> uvs;
	meshPoints.Add(FVector(0.0f, -100.0f, 500.0f));
	uvs.Add(FVector2D(0.0f, 1.0f));
	meshPoints.Add(FVector(0.0f, 100.0f, 500.0f));
	uvs.Add(FVector2D(1.0f, 1.0f));
	meshPoints.Add(FVector(0.0f, -100.0f, 0.0f));
	uvs.Add(FVector2D(0.0f, 0.0f));
	meshPoints.Add(FVector(0.0f, 100.0f, 0.0f));
	uvs.Add(FVector2D(1.0f, 0.0f));

	for(int i = 0; i < meshPoints.Num(); i++){
		pointToOriginalMap.Add(i);
	}

	meshWedges.AddDefaulted_GetRef().iVertex = 0;
	meshWedges.AddDefaulted_GetRef().iVertex = 2;
	meshWedges.AddDefaulted_GetRef().iVertex = 1;
	meshWedges.AddDefaulted_GetRef().iVertex = 0;
	meshWedges.AddDefaulted_GetRef().iVertex = 3;
	meshWedges.AddDefaulted_GetRef().iVertex = 2;
	for(int i = 0; i < meshWedges.Num(); i++){
		meshWedges[i].UVs[0] = uvs[meshWedges[i].iVertex];
	}

	auto* infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 0;
	infl->VertIndex = 0;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 0;
	infl->VertIndex = 1;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 1;
	infl->VertIndex = 2;
	infl->Weight = 1.0f;

	infl = &meshInfluences.AddDefaulted_GetRef();
	infl->BoneIndex = 1;
	infl->VertIndex = 3;
	infl->Weight = 1.0f;

	FVector norm(-1.0f, 0.0f, 0.0f);
	FVector tanU(0.0f, 1.0f, 0.0f);
	FVector tanV(0.0f, 0.0f, 1.0f);

	meshFaces.AddDefaulted();
	meshFaces.AddDefaulted();
	for(int faceIndex = 0; faceIndex < meshFaces.Num(); faceIndex++){
		for(int i = 0; i < 3; i++){
			meshFaces[faceIndex].iWedge[i] = i + faceIndex * 3;
			meshFaces[faceIndex].TangentX[i] = tanU;
			meshFaces[faceIndex].TangentY[i] = tanV;
			meshFaces[faceIndex].TangentZ[i] = norm;
		}
	}

	skelMesh->Skeleton = NewObject<USkeleton>();
	skelMesh->SetFlags(RF_Standalone|RF_Public);
	skelMesh->Skeleton->MergeAllBonesToBoneTree(skelMesh);
	skelMesh->PostLoad();

	IMeshUtilities::MeshBuildOptions buildOptions;
	buildOptions.bComputeNormals = false;
	buildOptions.bComputeTangents = false;
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
}
#endif
void SkinMeshTest::run(){
	FString meshName = TEXT("SkinMeshDebug");
	FString meshPath = TEXT("SkinMeshDebug");
	auto fullPath = FString::Printf(TEXT("%s/%s"), *meshPath, *meshName);

	auto skelMesh = createAssetObject<USkeletalMesh>(meshName, &meshPath, nullptr, 
		[&](auto arg){
			fillTestSkinMesh(arg);
		}, RF_Standalone|RF_Public);

	/*
	auto meshPkg = createPackage(fullPath);
	auto skelMesh = NewObject<USkeletalMesh>(meshPkg, *meshName, RF_Standalone|RF_Public);

	fillTestSkinMesh(skelMesh);

	skelMesh->PostLoad();

	if (meshPkg){
		skelMesh->MarkPackageDirty();
		FAssetRegistryModule::AssetCreated(meshPkg);
	}
	*/
}
