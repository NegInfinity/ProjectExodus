#include "JsonImportPrivatePCH.h"
#include "JsonAudioclip.h"
#include "macros.h"

using namespace JsonObjects;

void JsonAudioclip::load(JsonObjPtr data){
	JSON_GET_VAR(data, name);

	JSON_GET_VAR(data, id);
	JSON_GET_VAR(data, assetPath);
	JSON_GET_VAR(data, exportPath);

	JSON_GET_VAR(data, ambisonic);
	JSON_GET_VAR(data, channels);
	JSON_GET_VAR(data, frequency);
	JSON_GET_VAR(data, length);
	JSON_GET_VAR(data, loadInBackground);
	JSON_GET_VAR(data, loadType);
	JSON_GET_VAR(data, preloadAudioData);
	JSON_GET_VAR(data, samples);
}
