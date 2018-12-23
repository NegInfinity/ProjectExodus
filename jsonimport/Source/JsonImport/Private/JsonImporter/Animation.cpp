#include "JsonImportPrivatePCH.h"
#include "AnimationBuilder.h"

#include "JsonImporter.h"
#include "UnrealUtilities.h"

#include "AnimationBuilder.h"

#include "Runtime/Engine/Classes/Animation/AnimSequence.h"

using namespace UnrealUtilities;

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

void JsonImporter::processAnimators(ImportWorkData &workData, const JsonGameObject &gameObj, ImportedGameObject *parentObject, const FString &folderPath){
	for(const auto& animator: gameObj.animators){
		processAnimator(workData, gameObj, animator, parentObject, folderPath);
	}
}

void JsonImporter::processAnimator(ImportWorkData &workData, const JsonGameObject &gameObj, const JsonAnimator &jsonAnimator, 
		ImportedGameObject *parentObject, const FString &folderPath){

	auto animatorId = jsonAnimator.animatorControllerId;
	if (animatorId < 0){
		UE_LOG(JsonLog, Warning, TEXT("Animator controller not found on object %s (%d)"), *gameObj.name, gameObj.id);
		return;
	}

	auto skelId = jsonAnimator.skeletonId;
	if (skelId < 0){
		UE_LOG(JsonLog, Warning, TEXT("Animator skeleton not found on object %s (%d)"), *gameObj.name, gameObj.id);
		return;
	}

	workData.registerDelayedAnimController(skelId, animatorId);
}

void JsonImporter::processDelayedAnimators(ImportWorkData &workData){
	FScopedSlowTask delayedAnimProgress(workData.delayedAnimControllers.Num(), 
		LOCTEXT("Processing animator controllers", "Processing animator controllers"));

	for(const auto& i: workData.delayedAnimControllers){
		processDelayedAnimator(i.Key, i.Value);
		delayedAnimProgress.EnterProgressFrame();
	}
}

void JsonImporter::processDelayedAnimator(JsonId skelId, JsonId controllerId){
	UE_LOG(JsonLog, Log, TEXT("Processing animator: skelId: %d, controllerId: %d"), skelId, controllerId);
	if (skelId < 0){
		UE_LOG(JsonLog, Warning, TEXT("Skeleton not found while processing delayed animator %d(skel) %d(controller)"),
			skelId, controllerId);
		return;
	}

	if (controllerId < 0){
		UE_LOG(JsonLog, Warning, TEXT("Controller not found while processing delayed animator %d(skel) %d(controller)"),
			skelId, controllerId);
		return;
	}

	auto skeleton = getSkeletonObject(skelId);
	if (!skeleton){
		UE_LOG(JsonLog, Warning, TEXT("Could not find skeleton %d while processing delayed animators."), skelId);
		return;
	}

	JsonAnimatorController animController;
	if (!loadIndexedExternResource(animController, controllerId, externResources.animatorControllers)){
		UE_LOG(JsonLog, Warning, TEXT("Could not load anim controller %d while processing delayed animators."), controllerId);
		return;
	}

	auto controllerPath = FPaths::GetPath(animController.path);
	auto baseName = FPaths::GetBaseFilename(animController.path);

	auto animBaseName = FString::Printf(TEXT("%s_skel%d"), *baseName, skelId);

	auto clipDir = FString::Printf(TEXT("%s/%s"), *controllerPath, *animBaseName);

	for(const auto clipIndex: animController.animationIds){
		JsonAnimationClip animClip;
		if (!loadIndexedExternResource(animClip, clipIndex, externResources.animationClips)){
			UE_LOG(JsonLog, Warning, TEXT("Coudl not load animation clip %d while processing animation with skelId: %d; controllerId: %d"),
				clipIndex, skelId, controllerId);
		}

		AnimationBuilder animBuilder;

		UAnimSequence *newSeq = createAssetObject<UAnimSequence>(animClip.name, &clipDir, this, 
			[&](UAnimSequence *newSeq){
				newSeq->SetSkeleton(skeleton);
				animBuilder.buildAnimation(newSeq, skeleton, animClip);
			}, RF_Standalone|RF_Public
		);
		UE_LOG(JsonLog, Log, TEXT("Created anim clip at \"%s\""), *newSeq->GetPathName());
		//createAssetObject(
		//auto assetObj = createAssetObject(
	}	
}
