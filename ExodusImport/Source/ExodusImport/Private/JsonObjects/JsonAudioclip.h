#pragma once

#include "JsonTypes.h"

class JsonAudioclip{
public:
	FString name;
	int id = -1;
	FString assetPath;
	FString exportPath;

	bool ambisonic;
	int channels;
	int frequency;
	float length;
	bool loadInBackground;
	FString loadType;
	bool preloadAudioData;
	int samples;

	void load(JsonObjPtr data);
	JsonAudioclip() = default;
	JsonAudioclip(JsonObjPtr data){
		load(data);
	}
};