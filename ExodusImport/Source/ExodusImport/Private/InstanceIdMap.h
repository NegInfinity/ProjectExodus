#pragma once
#include "JsonTypes.h"

using InstanceId = int;
//using InstanceToIdMap = TMap<InstanceId, JsonId>;

class InstanceIdMap{
protected:
	using Map = TMap<InstanceId, JsonId>;
	Map map;
public:
	void clear(){
		map.Empty(0);
	}
	int size() const{
		return map.Num();
	}
	int num() const{
		return size();
	}
	bool isEmpty(){
		return size() == 0;
	}

	const JsonId* find(InstanceId instId) const{
		return map.Find(instId);
	}
	const JsonId* findIdByInstance(InstanceId instId) const{
		return map.Find(instId);
	}
	bool registerId(InstanceId instId, JsonId jsonId);
};
