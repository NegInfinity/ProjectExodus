#include "JsonImportPrivatePCH.h"
#include "ResId.h"

uint32 GetTypeHash(const ResId &id){
	return (uint32)id.toIndex();
}
