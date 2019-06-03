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
#include "Engine/Classes/Engine/SkeletalMesh.h"
#include "Runtime/Engine/Classes/Animation/Skeleton.h"

#include "JsonObjects/JsonMesh.h"
#include "UnrealUtilities.h"

#include "DesktopPlatformModule.h"
#include "MeshBuilder.h"
#include "SkeletalMeshBuilder.h"
#include "Classes/PhysicsEngine/BodySetup.h"

using namespace UnrealUtilities;
using namespace JsonObjects;

void JsonImporter::importStaticMesh(const JsonMesh &jsonMesh, int32 meshId){
	auto unrealMeshName = jsonMesh.makeUnrealMeshName();
	auto desiredDir = FPaths::GetPath(jsonMesh.path);
	auto mesh = createAssetObject<UStaticMesh>(unrealMeshName, &desiredDir, this, 
		[&](UStaticMesh *mesh){
			MeshBuilder meshBuilder;
			meshBuilder.setupStaticMesh(mesh, jsonMesh, [&](auto &materials){
				materials.Empty();
				for(auto matId: jsonMesh.materials){
					UMaterialInterface *material = loadMaterialInterface(matId);
					materials.Add(material);
				}
			});
		},
		[&](auto pkg, auto objName){
			return NewObject<UStaticMesh>(pkg, FName(*objName), RF_Standalone|RF_Public);
		}, RF_Standalone|RF_Public
	);

	if (mesh){
		auto meshPath = mesh->GetPathName();
		meshIdMap.Add(jsonMesh.id, meshPath);
	}
}

void JsonImporter::importSkeletalMesh(const JsonMesh &jsonMesh, int32 meshId){
	auto skelId = jsonMesh.defaultSkeletonId;
	auto foundSkeleton = getSkeleton(skelId);
	if (!foundSkeleton){
		UE_LOG(JsonLog, Warning, TEXT("Could not find skeleton %d"), skelId);
	}

	auto unrealMeshName = jsonMesh.makeUnrealMeshName() + TEXT("_Skin");
	auto desiredDir = FPaths::GetPath(jsonMesh.path);

	USkeleton *skeleton = nullptr;

	auto mesh = createAssetObject<USkeletalMesh>(unrealMeshName, &desiredDir, this, 
		[&](USkeletalMesh *mesh){
			SkeletalMeshBuilder meshBuilder;
			meshBuilder.setupSkeletalMesh(mesh, jsonMesh, this, 
				[&](auto &materials){
					materials.Empty();
					for(auto matId: jsonMesh.materials){
						UMaterialInterface *material = loadMaterialInterface(matId);
						materials.Add(material);
					}
				},
				[&](const JsonSkeleton& jsonSkel, USkeleton *skel){
					check(skel);
					registerSkeleton(jsonSkel.id, skel);
				}
			);
		},
		[&](auto pkg, auto objName){
			return NewObject<USkeletalMesh>(pkg, FName(*objName), RF_Standalone|RF_Public);
		}, RF_Standalone|RF_Public
	);

	if (mesh){
		auto meshPath = mesh->GetPathName();
		skinMeshIdMap.Add(jsonMesh.id, meshPath);
	}
}

void JsonImporter::importMesh(const JsonMesh &jsonMesh, int32 meshId){
	UE_LOG(JsonLog, Log, TEXT("Importing mesh: %s(%d)"), *jsonMesh.name, jsonMesh.id.id)
	UE_LOG(JsonLog, Log, TEXT("Mesh data: Verts: %d; submeshes: %d; materials: %d; colors %d; normals: %d"), 
		jsonMesh.verts.Num(), jsonMesh.subMeshes.Num(), jsonMesh.colors.Num(), jsonMesh.normals.Num());
	UE_LOG(JsonLog, Log, TEXT("Mesh data: uv0: %d; uv1: %d; uv2: %d; uv3: %d; uv4: %d; uv5: %d; uv6: %d; uv7: %d;"),
		jsonMesh.uv0.Num(), jsonMesh.uv1.Num(), jsonMesh.uv2.Num(), jsonMesh.uv3.Num(), 
		jsonMesh.uv4.Num(), jsonMesh.uv5.Num(), jsonMesh.uv6.Num(), jsonMesh.uv7.Num());
	
	if (jsonMesh.verts.Num() <= 0){
		UE_LOG(JsonLog, Warning, TEXT("No verts, cannot create mesh!"));
		return;
	}

	importStaticMesh(jsonMesh, meshId);

	if (jsonMesh.hasBlendShapes() || jsonMesh.hasBoneWeights()){
		importSkeletalMesh(jsonMesh, meshId);
	}
}

void JsonImporter::importMesh(JsonObjPtr obj, int32 meshId){
	UE_LOG(JsonLog, Log, TEXT("Importing mesh %d"), meshId);

	JsonMesh jsonMesh(obj);

	importMesh(jsonMesh, meshId);
}

JsonMesh JsonImporter::loadJsonMesh(int32 id) const{
	if ((id < 0) || (id >= externResources.meshes.Num())){
		UE_LOG(JsonLog, Error, TEXT("Invalid mesh index %d, %d meshes total"), id, externResources.meshes.Num());
		return JsonMesh();
	}

	auto meshData = loadExternResourceFromFile(externResources.meshes[id]);
	return JsonMesh(meshData);
}

const JsonSkeleton* JsonImporter::getSkeleton(int32 id) const{
	return jsonSkeletons.Find(id);
}
