// Fill out your copyright notice in the Description page of Project Settings.


#include "UShaderBlueprintFunctionLibrary.h"

#include "CoreMinimal.h"
#include "Misc/CoreMisc.h"

#include "Shader.h"
#include "ShaderCodeLibrary.h"
#include "Engine.h"
#include "UObject/Class.h"
#include "UObject/Object.h"
#include "IAssetRegistry.h"
#include "AssetRegistryModule.h"
DEFINE_LOG_CATEGORY(MyLog);

void UShaderBlueprintFunctionLibrary::PrintShaderPath() {
	FJsonSerializableArray jsonSeriArr;
	GetAllVirtualShaderSourcePaths(jsonSeriArr, EShaderPlatform::SP_OPENGL_PCES3_1);
	if (jsonSeriArr.Num() > 0) {
		UE_LOG(MyLog, Log, TEXT("$$ shader source path count:%i"), jsonSeriArr.Num());

		for (auto b = jsonSeriArr.begin(), e = jsonSeriArr.end(); b != e; ++b) {
			FString pathStr;
			pathStr.Append((*b).GetCharArray());
			UE_LOG(MyLog, Log, TEXT("$$ shader path:%s"), *pathStr);
		}
	}
	//LoadShaderSourceFileChecked()

	bool rootFolder = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), TEXT(""));
	bool bFileFolder = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), TEXT("firstPerson"));
	bool bContentFolder = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), FString(TEXT("Game")).Append("/").Append(TEXT("Content")));
	
	UE_LOG(MyLog, Log, TEXT("%s"), (rootFolder ? TEXT("$$ load hotta shader from rootFolder") : TEXT("$$ couldn't load hotta shader from rootFolder")));
	UE_LOG(MyLog, Log, TEXT("%s"), (bFileFolder ? TEXT("$$ load hotta shader from FileFolder") : TEXT("$$ couldn't load hotta shader from FileFolder")));
	UE_LOG(MyLog, Log, TEXT("%s"), (bContentFolder ? TEXT("$$ load hotta shader from ContentFolder") : TEXT("$$ couldn't load hotta shader from ContentFolder")));

	GetAllVirtualShaderSourcePaths(jsonSeriArr, EShaderPlatform::SP_OPENGL_PCES3_1);
	UE_LOG(MyLog, Log, TEXT("$$ shader source path count: %i"), jsonSeriArr.Num());


	if (GEngine != NULL) {
		auto pak = LoadPackage(NULL, TEXT(""), ELoadFlags::LOAD_None);
		if (pak != nullptr) {
			UE_LOG(MyLog, Log, TEXT("$$ get asset bundle.."), TEXT("")); 
		}
		auto pkg = LoadObject<UPackage>(nullptr, TEXT("firstPerson-Android_ASTC0"));
		UE_LOG(MyLog, Log, TEXT("$$ asset bundle %s.."), pkg!= nullptr ? TEXT("exist") : TEXT("not exist"));

		FAssetRegistryModule& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		IAssetRegistry& assetRegistry = assetRegistryModule.Get();

		const FAssetPackageData* assetPkgData = assetRegistry.GetAssetPackageData("firstPerson-Android_ASTC0");
		UE_LOG(MyLog, Log, TEXT("$$ assetPkgData %s"), assetPkgData != nullptr ? TEXT("exist") : TEXT("not exist"));

		TArray<FAssetData> assets;
		assetRegistry.GetAllAssets(assets);

		for (auto begin = assets.begin(), end = assets.end(); begin != end; ++begin) {
			auto asset = (*begin);
			//auto package = asset.GetPackage();
			UE_LOG(MyLog, Log, TEXT("$$ assetName:%s"), *asset.AssetName.ToString());
			
		}

	}

}

void test() {
	
}