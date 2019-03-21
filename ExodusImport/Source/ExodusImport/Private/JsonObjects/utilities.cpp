#include "JsonImportPrivatePCH.h"
#include "utilities.h"

using namespace JsonObjects;

bool JsonObjects::isValidId(JsonId id){
	return id >= 0;
}

bool JsonObjects::isValidId(const ResId &id){
	return id.isValid();
}

FLinearColor JsonObjects::applyGamma(const FLinearColor &arg){
	return FLinearColor(powf(arg.R, 2.2f), powf(arg.G, 2.2f), pow(arg.B, 2.2f), arg.A);
}

