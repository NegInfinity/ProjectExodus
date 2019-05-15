#pragma once

#include "JsonTypes.h"
#include <functional>

#include "Runtime/Engine/Classes/Engine/SkeletalMesh.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshModel.h"
#include "JsonObjects/JsonMesh.h"
#include "JsonObjects/JsonSkeleton.h"

class UStaticMesh;
class USkeletalMesh;
class UMaterial;
class UMaterialInterface;
class JsonImporter;

struct SkeletalMeshBuildData{
	bool hasColors = false;
	bool hasNormals = false;
	bool hasTangents = false;

	TArray<UMorphTarget*> morphTargets;

	TArray<SkeletalMeshImportData::FVertInfluence> meshInfluences;
	TArray<SkeletalMeshImportData::FMeshWedge> meshWedges;
	TArray<SkeletalMeshImportData::FMeshFace> meshFaces;
	TArray<FVector> meshPoints;
	TArray<int32> pointToOriginalMap;
	TArray<FText> buildWarnMessages;
	TArray<FName> buildWarnNames;

	void startWithMesh(const JsonMesh &jsonMesh);
	void processPositionsAndWeights(const JsonMesh &jsonMesh, const TMap<int, int> &meshToSkeletonBoneMap, StringArray &remapErrors);
	void processWedgeData(const JsonMesh &jsonMesh);

	void buildSkeletalMesh(FSkeletalMeshLODModel &lodModel, const FReferenceSkeleton &refSkeleton, const JsonMesh &jsonMesh);
	void computeBoundingBox(USkeletalMesh *skelMesh, const JsonMesh &jsonMesh);

	void processBlendShapes(USkeletalMesh *skelMesh, const JsonMesh &jsonMesh);
};

class SkeletalMeshBuilder{
public:
	void setupSkeletalMesh(USkeletalMesh *mesh, const JsonMesh &jsonMesh, const JsonImporter *importer, 
		std::function<void(TArray<FSkeletalMaterial> &meshMaterials)> materialSetup, 
		std::function<void(const JsonSkeleton&, USkeleton*)> onNewSkeleton);
protected:
	void setupReferenceSkeleton(FReferenceSkeleton &refSkeleton, const JsonSkeleton &jsonSkel, const JsonMesh *jsonMesh,  const USkeleton *unrealSkeleton) const;
	void registerPreviewMesh(USkeleton *skel, USkeletalMesh *mesh, const JsonMesh &jsonMesh);
};
