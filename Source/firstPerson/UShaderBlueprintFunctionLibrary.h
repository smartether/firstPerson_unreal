// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UShaderBlueprintFunctionLibrary.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(MyLog, Log, All)

UCLASS()
class UAssetLoadedCallback : public UObject
{
	GENERATED_BODY()
public:
	void OnCreateAllChildren();
	UAssetLoadedCallback();
};

/**
 * 
 */
UCLASS(Blueprintable)
class FIRSTPERSON_API UShaderBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	static UAssetLoadedCallback* callback;
	static TArray<FSoftObjectPath>* ObjectPaths;
	static TArray<TSoftObjectPtr<UObject>>* ObjectPtrs;
	static TSharedPtr<FStreamableHandle> streamableHandlePtr;

	static TArray<UObject*>* Actors;
	 
public:	

	UFUNCTION(BlueprintCallable, Category="ShaderBytecode")
	static void  PrintShaderPath();

	static TArray<FSoftObjectPath>* GetObjectPaths();
	static TArray<TSoftObjectPtr<UObject>>* GetObjectPtrs();
	static TSharedPtr<FStreamableHandle> GetStreamableHandle();
	static TArray<UObject*>* GetActors();
	
};

