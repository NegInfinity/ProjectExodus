#pragma once
#include "CoreMinimal.h"
#include "JsonObjects.h"

class AnimationBuilder{
public:
	void buildAnimation(UAnimSequence *animSequence, USkeleton *skeleton, const JsonAnimationClip &srcClip);
};
