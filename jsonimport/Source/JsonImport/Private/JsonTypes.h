#pragma once

#include "CoreMinimal.h"
#include "JsonLog.h"

using FStringArray = TArray<FString>;
using JsonObjPtr = TSharedPtr<FJsonObject>;
using JsonValPtr = TSharedPtr<FJsonValue>;
using JsonReaderRef = TSharedRef<TJsonReader<>>;
using JsonObjPtrs = TArray<JsonObjPtr>;
using JsonValPtrs = TArray<JsonValPtr>;
using IdNameMap = TMap<int, FString>;

using JsonId = int;
using JsonTextureId = JsonId;
using JsonMeshId = JsonId;
using JsonMaterialId = JsonId;
using JsonObjectId = JsonId;

using IntArray = TArray<int32>;
using FloatArray = TArray<float>;

