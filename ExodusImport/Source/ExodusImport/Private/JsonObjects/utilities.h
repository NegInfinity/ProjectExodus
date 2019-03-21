#pragma once

#include "JsonTypes.h"

namespace JsonObjects{
	bool isValidId(JsonId id);	
	bool isValidId(const ResId &id);
	FLinearColor applyGamma(const FLinearColor &arg);
}
