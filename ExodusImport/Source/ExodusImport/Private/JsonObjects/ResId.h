#pragma once
#include "JsonTypes.h"

struct ResId{
public:
	int id = -1;

	bool isValid() const{
		return id >= 0;
	}

	int toIndex() const{
		return id;
	}

	explicit operator int() const{
		return id;
	}

	/*
	bool operator<(const ResId& other) const{
		return id < other.id;
	}
	*/

	bool operator==(const ResId& other) const{
		return id == other.id;
	}

	bool operator!=(const ResId& other) const{
		return id != other.id;
	}

	static ResId fromIndex(int index){
		return ResId{index};
		//ResId result = {index};
		//return result;
		//return ResId(){index};
	}

	ResId() = default;
};

uint32 GetTypeHash(const ResId &id);
