#include "JsonImportPrivatePCH.h"
#include "AnimationBuilder.h"
#include "Runtime/Engine/Classes/Animation/AnimSequence.h"
#include "Runtime/Engine/Classes/Animation/Skeleton.h"

void addRawTrackBoneKey(FRawAnimSequenceTrack &outTrack, const JsonTransformKey &key){
	auto unrealMatrix = key.local.getUnrealTransform();
	FTransform transform;
	transform.SetFromMatrix(unrealMatrix);

	outTrack.PosKeys.Add(transform.GetLocation());
	outTrack.ScaleKeys.Add(transform.GetScale3D());
	outTrack.RotKeys.Add(transform.GetRotation());
}

void AnimationBuilder::buildAnimation(UAnimSequence *animSeq, USkeleton *skel, const JsonAnimationClip &srcClip){
	check(animSeq);
	animSeq->CleanAnimSequenceForImport();
	if (!skel){
		skel = animSeq->GetSkeleton();
	}
	//USkeleton *skel = animSeq->GetSkeleton();
	check(skel);

	int minFrame = 0;
	int maxFrame = 0;

	for(const auto &matCurve: srcClip.matrixCurves){
		if (matCurve.keys.Num() <= 0)
			continue;

		const auto &firstKey = matCurve.keys[0];
		const auto &lastKey = matCurve.keys[matCurve.keys.Num() - 1];
		minFrame = FMath::Min(minFrame, firstKey.frame);
		maxFrame = FMath::Max(maxFrame, lastKey.frame);
	}

	UE_LOG(JsonLog, Log,  TEXT(""));

	int numFrames = maxFrame - minFrame + 1;

	for(const auto &matCurve: srcClip.matrixCurves){
		if (matCurve.keys.Num() <= 0)
			continue;

		FRawAnimSequenceTrack rawAnimTrack;

		const auto &firstKey = matCurve.keys[0];
		const auto &lastKey = matCurve.keys[matCurve.keys.Num() - 1];
		int lastFrame = -1;
		
		int frameIndex = 0;
		while(frameIndex < firstKey.frame){
			addRawTrackBoneKey(rawAnimTrack, firstKey);
			frameIndex++;
		}

		auto lastWrittenKey = firstKey;
		for(int i = 0; i < matCurve.keys.Num(); i++){
			const auto &curKey = matCurve.keys[i];
			while(frameIndex < curKey.frame){
				addRawTrackBoneKey(rawAnimTrack, lastWrittenKey);
				frameIndex++;
			}
			lastWrittenKey = curKey;
			addRawTrackBoneKey(rawAnimTrack, curKey);
			frameIndex++;
		}

		while(frameIndex <= maxFrame){
			addRawTrackBoneKey(rawAnimTrack, lastKey);
			frameIndex++;
		}

		animSeq->AddNewRawTrack(*matCurve.objectName, &rawAnimTrack);
	}

	animSeq->SetRawNumberOfFrame(numFrames);
	//animSeq->NumFrames = numFrames;
	//aww, hell. No "fps" here...
	float frameRate = srcClip.frameRate ? srcClip.frameRate : 30.0f;
	animSeq->SequenceLength = (float)numFrames / frameRate;
	animSeq->MarkRawDataAsModified();

	//animSeq->RawCurveData.

	//Hmm. Things that would interest us are blendshapes and matrix keys.
	/*
	Relevant information is in SkeletalMeshEdito.cpp....
	*/
}

