#include "JsonImportPrivatePCH.h"
#include "AnimationBuilder.h"
#include "Runtime/Engine/Classes/Animation/AnimSequence.h"
#include "Runtime/Engine/Classes/Animation/Skeleton.h"

void AnimationBuilder::buildAnimation(UAnimSequence *animSeq, const JsonAnimationClip &srcClip){
	check(animSeq);
	animSeq->CleanAnimSequenceForImport();
	USkeleton *skel = animSeq->GetSkeleton();
	check(skel);

	//animSeq->RawCurveData.

	//Hmm. Things that would interest us are blendshapes and matrix keys.
	/*
	Relevant information is in SkeletalMeshEdito.cpp....
	*/
}

