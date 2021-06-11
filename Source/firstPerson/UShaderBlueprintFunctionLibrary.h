// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UShaderBlueprintFunctionLibrary.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(MyLog, Log, All)
/**
 * 
 */
UCLASS(Blueprintable)
class FIRSTPERSON_API UShaderBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="ShaderBytecode")
	static void  PrintShaderPath();
};
