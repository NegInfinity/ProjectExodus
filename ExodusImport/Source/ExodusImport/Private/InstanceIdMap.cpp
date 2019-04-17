#include "JsonImportPrivatePCH.h"
#include "InstanceIdMap.h"
#include "JsonLog.h"

bool InstanceIdMap::registerId(InstanceId instId, JsonId jsonId){
	auto found = map.Find(instId);
	if (found){
		UE_LOG(JsonLog, Warning, TEXT("Duplicate id (%d) found while registering instanceid %d. Existing id %d"),
			jsonId, instId, *found);

		map[instId] = jsonId;
		return false;
	}
	map.Add(instId, jsonId);
	return true;
}

