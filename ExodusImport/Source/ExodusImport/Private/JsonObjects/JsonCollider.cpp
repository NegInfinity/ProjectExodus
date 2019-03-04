#include "JsonImportPrivatePCH.h"
#include "JsonCollider.h"
#include "macros.h"

JsonCollider::JsonCollider(JsonObjPtr data){
	load(data);
}

void JsonCollider::load(JsonObjPtr data){
	using namespace JsonObjects;

	JSON_GET_VAR(data, colliderType);
	JSON_GET_VAR(data, colliderIndex);
	JSON_GET_VAR(data, center);
	JSON_GET_VAR(data, size);
	JSON_GET_VAR(data, direction);
	JSON_GET_VAR(data, radius);
	JSON_GET_VAR(data, height);
	JSON_GET_VAR(data, meshId);

	JSON_GET_VAR(data, enabled);
	JSON_GET_VAR(data, contactOffset);

	getJsonObj(data, bounds, "bounds");

	getJsonValue(trigger, data, "isTrigger");
	//JSON_GET_VAR(data, trigger);
}
