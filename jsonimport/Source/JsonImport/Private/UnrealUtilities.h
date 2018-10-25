#pragma once

#include "CoreMinimal.h"
#include "JsonTypes.h"
#include "JsonLog.h"
#include "ImportWorkData.h"

namespace UnrealUtilities{
	template <typename T>T* createActor(UWorld *world, FTransform transform, bool editorMode, const TCHAR* logName = 0){
		T* result = 0;
		if (editorMode){
			result = 	Cast<T>(GEditor->AddActor(world->GetCurrentLevel(),
				T::StaticClass(), transform));
		}
		else{
			result = world->SpawnActor<T>();
		}
		if (!result){
			if (logName){
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn actor %s"), logName);
			}
			else{
				UE_LOG(JsonLog, Warning, TEXT("Could not spawn templated actor"));
			}
		}
		else{
			auto moveResult = result->SetActorTransform(transform, false, nullptr, ETeleportType::ResetPhysics);
			logValue("Actor move result: ", moveResult);
		}
		return result;
	}

	template <typename T>T* createActor(ImportWorkData& workData, FTransform transform, const TCHAR* logName = 0){
		return createActor<T>(workData.world, transform, workData.editorMode, logName);
	}	
}