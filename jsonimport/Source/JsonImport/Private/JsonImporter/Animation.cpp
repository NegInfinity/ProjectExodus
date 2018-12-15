#include "JsonImportPrivatePCH.h"

#include "JsonImporter.h"


JsonAnimationClip JsonImporter::loadAnimationClip(JsonId id) const{
	check((id >= 0) && (id < this->externResources.animationClips.Num()));
	return JsonAnimationClip();
}

JsonAnimatorController JsonImporter::loadAnimationController(JsonId id) const{
	check((id >= 0) && (id < this->externResources.animatorControllers.Num()));
	return JsonAnimatorController();
}

//	void loadAnimatorsDebug(const StringArray &animatorPaths);
//	void loadAnimClipsDebug(const StringArray &animClipPaths);

void JsonImporter::loadAnimatorsDebug(const StringArray &animatorPaths){
	//const auto &animatorPaths = externResources.animatorControllers;
	UE_LOG(JsonLog, Log, TEXT("Debug loading animators. %d animators total"), animatorPaths.Num());
	for(int i = 0; i < animatorPaths.Num(); i++){
		auto curPath = animatorPaths[i];
		UE_LOG(JsonLog, Log, TEXT("Loading animator %d (path: %s)"), i, *curPath);

		auto dataPtr = loadExternResourceFromFile(curPath);
		if (!dataPtr){
			UE_LOG(JsonLog, Warning, TEXT("Load filed for animator clip %d, path %s"), i, *curPath);
			continue;
		}
		auto animator = JsonAnimatorController(dataPtr);
		UE_LOG(JsonLog, Log, TEXT("Animator loaded: %s"), *animator.name);
	}
}

void JsonImporter::loadAnimClipsDebug(const StringArray &animClipPaths){
	//const auto &animClipPaths = externResources.animationClips;
	UE_LOG(JsonLog, Log, TEXT("Debug loading animators. %d animators total"), animClipPaths.Num());
	for(int i = 0; i < animClipPaths.Num(); i++){
		auto curPath = animClipPaths[i];
		UE_LOG(JsonLog, Log, TEXT("Loading animation clip %d (path: %s)"), i, *curPath);
		//JsonObjPtr loadExternResourceFromFile(const FString &filename) const;
		auto dataPtr = loadExternResourceFromFile(curPath);
		if (!dataPtr){
			UE_LOG(JsonLog, Warning, TEXT("Load filed for animator clip %d, path %s"), i, *curPath);
			continue;
		}
		//auto cur = JsonAnimationClip(
		auto animClip = JsonAnimationClip(dataPtr);
		UE_LOG(JsonLog, Log, TEXT("Animation clip loaded: %s"), *animClip.name);
	}
}
