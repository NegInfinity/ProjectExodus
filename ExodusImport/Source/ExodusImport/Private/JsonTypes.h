#pragma once

#include "CoreMinimal.h"
#include "JsonLog.h"
#include "JsonObjects/ResId.h"

using FStringArray = TArray<FString>;
using JsonObjPtr = TSharedPtr<FJsonObject>;
using JsonValPtr = TSharedPtr<FJsonValue>;
using JsonReaderRef = TSharedRef<TJsonReader<>>;
using JsonObjPtrs = TArray<JsonObjPtr>;
using JsonValPtrs = TArray<JsonValPtr>;
using IdNameMap = TMap<int, FString>;
using ResIdNameMap = TMap<ResId, FString>;

using JsonId = int;
using InstanceId = int;
using JsonTextureId = JsonId;
using JsonMeshId = JsonId;
using JsonMaterialId = JsonId;
using JsonObjectId = JsonId;

using InstanceToIdMap = TMap<InstanceId, JsonId>;

using IntArray = TArray<int32>;
using FloatArray = TArray<float>;
using ByteArray = TArray<uint8>;
using StringArray = TArray<FString>;
using LinearColorArray = TArray<FLinearColor>;
using MatrixArray = TArray<FMatrix>;
