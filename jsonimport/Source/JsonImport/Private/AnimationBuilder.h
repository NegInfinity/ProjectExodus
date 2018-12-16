#pragma once
#include "CoreMinimal.h"
#include "JsonObjects.h"

class AnimationBuilder{
public:
	void buildAnimation(UAnimSequence *animSequence, const JsonAnimationClip &srcClip);
};
