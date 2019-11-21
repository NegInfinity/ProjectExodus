#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"
#include "UnrealUtilities.h"
#include "builders/JointBuilder.h"
#include "builders/PrefabBuilder.h"

#include "LocTextNamespace.h"

#define LOCTEXT_NAMESPACE LOCTEXT_NAMESPACE_NAME

#define JSON_DISABLE_PREFAB_IMPORT

using namespace JsonObjects;
using namespace UnrealUtilities;

FString JsonImporter::getProjectImportPath() const{
	auto result = getDefaultImportPath();
	if (result.Len() && sourceBaseName.Len())
		result = FPaths::Combine(*result, *sourceBaseName);
	return result;
}

void JsonImporter::importTerrainData(JsonObjPtr jsonData, JsonId terrainId, const FString &rootPath){
	//
	JsonTerrainData terrainData;
	terrainData.load(jsonData);

	terrainDataMap.Add(terrainId, terrainData);
}

void JsonImporter::loadTerrains(const StringArray &terrains){
	FScopedSlowTask terProgress(terrains.Num(), LOCTEXT("Importing terrains", "Importing terrains"));
	terProgress.MakeDialog();
	JsonId id = 0;
	for(auto curFilename: terrains){
		auto obj = loadExternResourceFromFile(curFilename);//cur->AsObject();
		auto curId= id;
		id++;
		if (!obj.IsValid())
			continue;

		importTerrainData(obj, curId, assetRootPath);
		terProgress.EnterProgressFrame(1.0f);
	}

}

void JsonImporter::loadCubemaps(const StringArray &cubemaps){
	FScopedSlowTask texProgress(cubemaps.Num(), LOCTEXT("Importing cubemaps", "Importing cubemaps"));
	texProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing textures"));
	for(auto curFilename: cubemaps){
		auto obj = loadExternResourceFromFile(curFilename);
		if (!obj.IsValid())
			continue;
		importCubemap(obj, assetRootPath);
		texProgress.EnterProgressFrame(1.0f);
	}
}

void JsonImporter::loadTextures(const StringArray & textures){
	FScopedSlowTask texProgress(textures.Num(), LOCTEXT("Importing textures", "Importing textures"));
	texProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing textures"));
	for(auto curFilename: textures){
		auto obj = loadExternResourceFromFile(curFilename);
		if (!obj.IsValid())
			continue;
		importTexture(obj, assetRootPath);
		texProgress.EnterProgressFrame(1.0f);
	}
}

void JsonImporter::loadSkeletons(const StringArray &skeletons){
	FScopedSlowTask skelProgress(skeletons.Num(), LOCTEXT("Importing skeletons", "Importing skeletons"));
	skelProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing skeletons"));

	jsonSkeletons.Empty();

	for(int id = 0; id < skeletons.Num(); id++){
		const auto& curFilename = skeletons[id];
		auto obj = loadExternResourceFromFile(curFilename);
		if (!obj.IsValid()){
			continue;
		}

		JsonSkeleton jsonSkel(obj);
		jsonSkeletons.Add(id, jsonSkel);
		UE_LOG(JsonLog, Log, TEXT("Loaded json skeleotn #%d (%s)"), jsonSkel.id, *jsonSkel.name);

		skelProgress.EnterProgressFrame(1.0f);
	}
}

void JsonImporter::loadMaterials(const StringArray &materials){
	FScopedSlowTask matProgress(materials.Num(), LOCTEXT("Importing materials", "Importing materials"));
	matProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing materials"));
	jsonMaterials.Empty();
	int32 matId = 0;
	for(auto curFilename: materials){
		auto obj = loadExternResourceFromFile(curFilename);
		auto curId = matId;
		matId++;//hmm.... I should probably axe this?
		if (!obj.IsValid())
			continue;

		JsonMaterial jsonMat(obj);
		jsonMaterials.Add(jsonMat);
		//importMasterMaterial(obj, curId);
		if (!jsonMat.supportedShader){
			UE_LOG(JsonLog, Warning, TEXT("Material \"%s\"(id: %d) is marked as having unsupported shader \"%s\""),
				*jsonMat.name, jsonMat.id, *jsonMat.shader);
		}

		auto matInst = materialBuilder.importMaterialInstance(jsonMat, this);
		if (matInst){
			//registerMaterialInstancePath(curId, matInst->GetPathName());
			registerMaterialInstancePath(jsonMat.id, matInst->GetPathName());
		}

		//importMaterialInstance(jsonMat, curId);
		matProgress.EnterProgressFrame(1.0f);
	}
}

void JsonImporter::loadMeshes(const StringArray &meshes){
	FScopedSlowTask meshProgress(meshes.Num(), LOCTEXT("Importing materials", "Importing meshes"));
	meshProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Processing meshes"));
	int32 meshId = 0;
	for(auto curFilename: meshes){
		auto obj = loadExternResourceFromFile(curFilename);
		auto curId = meshId;
		meshId++;//and this one too...
		if (!obj.IsValid())
			continue;
		importMesh(obj, curId);
		meshProgress.EnterProgressFrame(1.0f);
	}
}

void JsonImporter::loadObjects(const TArray<JsonGameObject> &objects, ImportContext &importData){
	FScopedSlowTask objProgress(objects.Num(), LOCTEXT("Importing objects", "Importing objects"));
	objProgress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Import objects"));
	//int32 objId = 0;
	for(const auto &curObj: objects){
		//auto curId = objId;
		//objId++;
		importObject(curObj, importData);
		objProgress.EnterProgressFrame(1.0f);
	}

	JointBuilder jointBuilder;
	jointBuilder.processPhysicsJoints(objects, importData);
	processDelayedAnimators(objects, importData);
}

void JsonImporter::importResources(const JsonExternResourceList &externRes){
	assetCommonPath = findCommonPath(externRes.resources);

	loadTextures(externRes.textures);
	loadCubemaps(externRes.cubemaps);
	loadMaterials(externRes.materials);
	loadSkeletons(externRes.skeletons);
	loadMeshes(externRes.meshes);
	importPrefabs(externRes.prefabs);
	loadTerrains(externRes.terrains);

	//loadAnimClipsDebug(externRes.animationClips);
	//loadAnimatorsDebug(externRes.animatorControllers); 
}

JsonObjPtr JsonImporter::loadExternResourceFromFile(const FString &filename) const{
	auto fullPath = FPaths::Combine(sourceExternDataPath, filename);
	return loadJsonFromFile(fullPath);
}

void JsonImporter::setupAssetPaths(const FString &jsonFilename){
	assetRootPath = FPaths::GetPath(jsonFilename);
	sourceBaseName = FPaths::GetBaseFilename(jsonFilename);
	sourceExternDataPath = FPaths::Combine(*assetRootPath, *sourceBaseName);
	assetRootPath = sourceExternDataPath;
}

void JsonImporter::registerMasterMaterialPath(int32 id, FString path){
	if (matMasterIdMap.Contains(id)){
		UE_LOG(JsonLog, Warning, TEXT("DUplicate material registration for id %d, path \"%s\""), id, *path);
	}
	matMasterIdMap.Add(id, path);
}

void JsonImporter::registerEmissiveMaterial(int32 id){
	emissiveMaterials.Add(id);
}

FString JsonImporter::getMeshPath(ResId id) const{
	auto result = meshIdMap.Find(id);
	if (result)
		return *result;
	return FString();
}

UStaticMesh* JsonImporter::loadStaticMeshById(ResId id) const{
	auto path = meshIdMap.Find(id);
	if (!path)
		return nullptr;
	auto result = LoadObject<UStaticMesh>(0, **path);
	return result;
}

void JsonImporter::registerMaterialInstancePath(int32 id, FString path){
	if (matInstIdMap.Contains(id)){
		UE_LOG(JsonLog, Warning, TEXT("Duplicate material registration for id %d, path \"%s\""), id, *path);
	}
	matInstIdMap.Add(id, path);
}

UMaterialInterface* JsonImporter::loadMaterialInterface(int32 id) const{
	return loadMaterialInstance(id);
}


USkeleton* JsonImporter::getSkeletonObject(int32 id) const{
	auto found = skeletonIdMap.Find(id);
	if (!found)
		return nullptr;
	auto result = LoadObject<USkeleton>(nullptr, **found);
	return result;
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

UAnimSequence* JsonImporter::getAnimSequence(AnimClipIdKey key) const{
	auto found = animClipPaths.Find(key);
	if (!found)
		return nullptr;
	return LoadObject<UAnimSequence>(nullptr, **found);
}

void JsonImporter::registerAnimSequence(AnimClipIdKey key, UAnimSequence *sequence){
	check(sequence);
	if (animClipPaths.Contains(key)){
		UE_LOG(JsonLog, Warning, TEXT("Duplicate animation clip regsitration for skeleton %d; clip %d"), key.Key, key.Value)
		return;
	}
	auto path = sequence->GetPathName();
	animClipPaths.Add(key, path);
}

const FString* JsonImporter::findMeshPath(ResId meshId) const{
	return meshIdMap.Find(meshId);
}

void JsonImporter::importPrefabs(const StringArray &prefabs){
#ifdef JSON_DISABLE_PREFAB_IMPORT
	UE_LOG(JsonLog, Warning, TEXT("Prefab import is currently disabled"));
	return;
#endif

	FScopedSlowTask progress(prefabs.Num(), LOCTEXT("Importing prefabs", "Importing prefabs"));
	progress.MakeDialog();
	UE_LOG(JsonLog, Log, TEXT("Import prefabs"));
	int32 objId = 0;
	for(auto curFilename: prefabs){
		auto obj = loadExternResourceFromFile(curFilename);
		auto curId = objId;
		objId++;
		if (!obj.IsValid())
			continue;

		PrefabBuilder builder;
		auto prefab = JsonPrefabData(obj);

		builder.importPrefab(prefab, this);
		//importPrefab(prefab);

		progress.EnterProgressFrame(1.0f);
	}
}

#undef LOCTEXT_NAMESPACE
