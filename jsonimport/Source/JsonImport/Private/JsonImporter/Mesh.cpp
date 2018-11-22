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

#include "JsonObjects/JsonMesh.h"
#include "UnrealUtilities.h"
#include "JsonObjects/converters.h"

#include "DesktopPlatformModule.h"
#include "MeshBuilder.h"

using namespace UnrealUtilities;
using namespace JsonObjects;

static FVector2D getIdxVector2(const TArray<float>& floats, int32 idx){
	if (floats.Num() <= (idx*2 + 1))
		return FVector2D();
	return FVector2D(floats[idx*2], floats[idx*2+1]);
};

static FVector getIdxVector3(const TArray<float>& floats, int32 idx){
	if (floats.Num() <= (idx*3 + 2))
		return FVector();
	return FVector(floats[idx*3], floats[idx*3+1], floats[idx*3+2]);
};

void JsonImporter::importMesh(JsonObjPtr obj, int32 meshId){
	UE_LOG(JsonLog, Log, TEXT("Importing mesh %d"), meshId);

	JsonMesh jsonMesh(obj);

	UE_LOG(JsonLog, Log, TEXT("Mesh data: Verts: %d; submeshes: %d; materials: %d; colors %d; normals: %d"), 
		jsonMesh.verts.Num(), jsonMesh.subMeshes.Num(), jsonMesh.colors.Num(), jsonMesh.normals.Num());
	UE_LOG(JsonLog, Log, TEXT("Mesh data: uv0: %d; uv1: %d; uv2: %d; uv3: %d; uv4: %d; uv5: %d; uv6: %d; uv7: %d;"),
		jsonMesh.uv0.Num(), jsonMesh.uv1.Num(), jsonMesh.uv2.Num(), jsonMesh.uv3.Num(), 
		jsonMesh.uv4.Num(), jsonMesh.uv5.Num(), jsonMesh.uv6.Num(), jsonMesh.uv7.Num());
	
	if (jsonMesh.verts.Num() <= 0){
		UE_LOG(JsonLog, Warning, TEXT("No verts, cannot create mesh!"));
		return;
	}

	FString sanitizedMeshName;
	FString sanitizedPackageName;

	FString meshName = jsonMesh.name;
	auto pathBaseName = FPaths::GetBaseFilename(jsonMesh.path);
	if (!pathBaseName.IsEmpty()){
		//Well, I've managed to create a level with two meshes named "cube". So...
		meshName = FString::Printf(TEXT("%s_%s_%d"), *pathBaseName, *meshName, meshId);
	}
	else{
		meshName = FString::Printf(TEXT("%s_%d"), *meshName, meshId);
	}

	UStaticMesh *existingMesh = nullptr;
	UPackage *meshPackage = createPackage(
		meshName, jsonMesh.path, assetRootPath, FString("Mesh"), 
		&sanitizedPackageName, &sanitizedMeshName, &existingMesh);
	if (existingMesh){
		meshIdMap.Add(jsonMesh.id, existingMesh->GetPathName());
		UE_LOG(JsonLog, Log, TEXT("Found existing mesh: %s (package %s)"), *sanitizedMeshName, *sanitizedPackageName);
		return;
	}

	UStaticMesh* mesh = NewObject<UStaticMesh>(meshPackage, FName(*sanitizedMeshName), RF_Standalone|RF_Public);

	if (!mesh){
		UE_LOG(JsonLog, Warning, TEXT("Couldn't create mesh"));
		return;
	}

	MeshBuilder meshBuilder;
	meshBuilder.setupMesh(mesh, jsonMesh, [&](auto &materials){
		materials.Empty();
		for(auto matId: jsonMesh.materials){
			UMaterialInterface *material = loadMaterialInterface(matId);
			materials.Add(material);
		}
	});

	if (mesh){
		auto meshPath = mesh->GetPathName();
		meshIdMap.Add(jsonMesh.id, meshPath);
		FAssetRegistryModule::AssetCreated(mesh);
		meshPackage->SetDirtyFlag(true);
	}
}

JsonMesh JsonImporter::loadJsonMesh(int32 id) const{
	if ((id < 0) || (id >= externResources.meshes.Num())){
		UE_LOG(JsonLog, Error, TEXT("Invalid mesh index %d, %d meshes total"), id, externResources.meshes.Num());
		return JsonMesh();
	}

	auto meshData = loadExternResourceFromFile(externResources.meshes[id]);
	return JsonMesh(meshData);
}
