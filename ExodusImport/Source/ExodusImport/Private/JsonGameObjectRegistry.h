#pragma once

#include "JsonTypes.h"
#include "InstanceIdMap.h"
#include "ImportedObject.h"

class JsonGameObjectRegistry{
protected:
	InstanceIdMap instanceIdMap;
	//InstanceToIdMap instanceIdMap;
	JsonGameObjectArray objects;

	void registerInstanceId(const JsonGameObject &obj);
	bool isValidIndex(int index) const;
public:
	using ConstIterator = JsonGameObjectArray::RangedForConstIteratorType;

	const JsonGameObject* resolveObjectReference(const JsonObjectReference &ref) const;

	const JsonGameObjectArray& getObjects() const{
		return objects;
	}

	const InstanceIdMap& getInstanceMap() const{
		return instanceIdMap;
	}

	const int getNumInstanceIds() const{
		return instanceIdMap.size();
	}

	/*
	ConstIterator begin() const{
		return objects.begin();
	}
	ConstIterator end() const{
		return objects.end();
	}
	*/

	void rebuildInstanceIdMap();
	int size() const{
		return objects.Num();
	}
	int num() const{
		return size();
	}
	bool siEmpty() const{
		return size() == 0;
	}
	void clear();
	//This function must be called in strictly sequential fashion!
	void addObject(const JsonGameObject &newObj);
	void buildFrom(const JsonGameObjectArray &objects);

	const JsonGameObject* findByInstanceId(InstanceId instId) const;
	const JsonGameObject* findByIndex(int index) const;
	const JsonGameObject& operator[](int index) const;
};
