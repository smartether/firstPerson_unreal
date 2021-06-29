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
#include "IPlatformFilePak.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Delegates/DelegateSignatureImpl.inl"

DEFINE_LOG_CATEGORY(MyLog);

void UAssetLoadedCallback::OnCreateAllChildren() {
	UE_LOG(MyLog, Log, TEXT("$$ assetLoaded"), "");
	auto objectPaths = UShaderBlueprintFunctionLibrary::GetObjectPaths();
	auto streamableHandle = UShaderBlueprintFunctionLibrary::GetStreamableHandle();

	//auto objectPtrs = UShaderBlueprintFunctionLibrary::GetObjectPtrs();
	TArray<UObject*> loadedObjects;
	streamableHandle->GetLoadedAssets(loadedObjects);
	//for (auto objectPath : *objectPaths) {
	//	
	//}
	for (auto objectPtr : loadedObjects) {
		if (objectPtr != nullptr) {
			UE_LOG(MyLog, Log, TEXT("$$ object loaded: %s"), *(objectPtr->GetFName().ToString()));

			if (objectPtr->GetFName() == TEXT("MM_Hotta_ToonUnlit")) {
				
				UMaterial* mm = reinterpret_cast<UMaterial*>(objectPtr);
				if (mm != nullptr) {
					
					//FVertexFactoryType* vertexFt = FVertexFactoryType::GetVFByName(FHashedName(TEXT("")));
					FLocalVertexFactory vertexF;
					auto matRes = mm->GetMaterialResource(ERHIFeatureLevel::ES3_1);
					auto shader = matRes->GetShader<FShaderType>(&vertexFt);
					
				}
			}
		}
	}
}

UAssetLoadedCallback::UAssetLoadedCallback() : UObject(){
	

}


UAssetLoadedCallback* UShaderBlueprintFunctionLibrary::callback = nullptr;
TArray<FSoftObjectPath>* UShaderBlueprintFunctionLibrary::ObjectPaths = nullptr;
TArray<TSoftObjectPtr<UObject>>* UShaderBlueprintFunctionLibrary::ObjectPtrs = nullptr;
TSharedPtr<FStreamableHandle> UShaderBlueprintFunctionLibrary::streamableHandlePtr = nullptr;
//
//TArray<FSoftObjectPath> UShaderBlueprintFunctionLibrary::ObjectPaths;
//TArray<TSoftObjectPtr<UObject>> UShaderBlueprintFunctionLibrary::ObjectPtrs;

TArray<FSoftObjectPath>* UShaderBlueprintFunctionLibrary::GetObjectPaths() {
	return UShaderBlueprintFunctionLibrary::ObjectPaths;
}

TArray<TSoftObjectPtr<UObject>>* UShaderBlueprintFunctionLibrary::GetObjectPtrs() {
	return UShaderBlueprintFunctionLibrary::ObjectPtrs;
}

TSharedPtr<FStreamableHandle> UShaderBlueprintFunctionLibrary::GetStreamableHandle() {
	return streamableHandlePtr;
}

void UShaderBlueprintFunctionLibrary::PrintShaderPath() {

	if (callback == nullptr) {
		callback = NewObject <UAssetLoadedCallback>();
		ObjectPaths = new TArray<FSoftObjectPath>();
		ObjectPtrs = new TArray<TSoftObjectPtr<UObject>>();
	}

	FJsonSerializableArray jsonArr;

	FPakPlatformFile::GetPakFolders(TEXT(""), jsonArr);

	for (auto begin = jsonArr.begin(), end = jsonArr.end(); begin != end; ++begin) {
		UE_LOG(MyLog, Log, TEXT("$$ pakFolder:%s"), *(*begin));
	}

	IPlatformFile& innerPlatform = FPlatformFileManager::Get().GetPlatformFile();
	FPakPlatformFile* pakPlatformFile = new FPakPlatformFile();
	pakPlatformFile->Initialize(&innerPlatform, TEXT(""));
	FPlatformFileManager::Get().SetPlatformFile(*pakPlatformFile);


	pakPlatformFile->MountAllPakFiles(jsonArr);

	const FString pakFileName = TEXT("/sdcard/pakchunk0-Android_ASTC.pak");
	FString MountPointEngine(FPaths::EngineContentDir());
	FString MountPointGame(FPaths::ProjectContentDir());
	FString MountRoot(FPaths::RootDir());

	
	
	FPakFile* pak = nullptr;// new FPakFile(&innerPlatform, *pakFileName, false);
	pak = reinterpret_cast<FPakFile*>(FCoreDelegates::MountPak.Execute(pakFileName, 4));
	
	
	if (pak->IsValid()) {
		UE_LOG(MyLog, Log, TEXT("$$ load pak success."), "");
		//pakPlatformFile->Mount(*pakFileName, 1000, *MountRoot);
		auto engineContentFolder = pak->GetMountPoint() + TEXT("Engine/Content/");
		UE_LOG(MyLog, Log, TEXT("$$ engineContentFolder:%s"), *engineContentFolder);
		FPackageName::RegisterMountPoint("/Engine/", engineContentFolder);

		auto gameContentFolder = pak->GetMountPoint() + TEXT("firstPerson/Content/");
		UE_LOG(MyLog, Log, TEXT("$$ contentFolder:%s"), *gameContentFolder);
		FPackageName::RegisterMountPoint("/Game/", gameContentFolder);
		
		if (!FShaderCodeLibrary::IsEnabled()) {
			UE_LOG(MyLog, Log, TEXT("$$ will init shaderCodeLib"), "");
			FShaderCodeLibrary::InitForRuntime(EShaderPlatform::SP_OPENGL_ES3_1_ANDROID);
		}


		auto bOpenShaderLib0 = FShaderCodeLibrary::OpenLibrary(TEXT("Global"), FPaths::ProjectContentDir());
		auto bOpenShaderLib01 = FShaderCodeLibrary::OpenLibrary(TEXT("Global"), TEXT("/Game/"));
		auto bOpenShaderLib1 = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), FPaths::ProjectContentDir());
		auto bOpenShaderLib2 = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), TEXT("/Game/"));
		
		UE_LOG(MyLog, Log, TEXT("$$ bOpenShaderLib0:%s"), (bOpenShaderLib0 ? TEXT("success") : TEXT("failed")));
		UE_LOG(MyLog, Log, TEXT("$$ bOpenShaderLib01:%s"), (bOpenShaderLib01 ? TEXT("success") : TEXT("failed")));
		UE_LOG(MyLog, Log, TEXT("$$ bOpenShaderLib1:%s"), (bOpenShaderLib1 ? TEXT("success") : TEXT("failed")));
		UE_LOG(MyLog, Log, TEXT("$$ bOpenShaderLib2:%s"), (bOpenShaderLib2 ? TEXT("success") : TEXT("failed")));
		
		//FJsonSerializableArray oldFolder;
		//FShaderCodeLibrary::CreatePatchLibrary(oldFolder, FPaths::ProjectContentDir(), TEXT(""), true, false);

		TArray<FString> files;
		pak->FindPrunedFilesAtPath(files, TEXT("../../../firstPerson/Content/AnimeToonShading"), true, false, true);
		//FJsonSerializableArray fileNames;
		//pak->GetPrunedFilenames(fileNames);
		//int maxPrintNum = 32;
		//for (auto fileName : fileNames) {
		//	if (maxPrintNum < 0)
		//		break;
		//	UE_LOG(MyLog, Log, TEXT("$$ fileName:%s"), *fileName);
		//	maxPrintNum--;
		//}

		for (auto file : files) {
			FString Filename, FileExtn, FileLongName;
			int32 LastSlashIndex;
			file.FindLastChar(*TEXT("/"), LastSlashIndex);
			FString FileOnly = file.RightChop(LastSlashIndex + 1);
			FileOnly.Split(TEXT("."), &Filename, &FileExtn);

			if (FileExtn == TEXT("uasset"))
			{
				FileLongName = FileOnly.Replace(TEXT("uasset"), *Filename);
			}

			//UE_LOG(MyLog, Log, TEXT("$$ file:%s"), *file);
			auto relPath = pakPlatformFile->ConvertToPakRelativePath(*file, pak);
			relPath = relPath.Replace(TEXT("firstPerson/Content/"), TEXT(""));
			UE_LOG(MyLog, Log, TEXT("$$ file rel1:%s"), *relPath);
			relPath = relPath.Replace(*FileOnly, TEXT("")) + FileLongName;
			UE_LOG(MyLog, Log, TEXT("$$ file rel2:%s"), *relPath);

			//FString relPathM = TEXT("/Engine/") + relPath;;
			//UShaderBlueprintFunctionLibrary::ObjectPaths->AddUnique(FSoftObjectPath(relPathM));
			//UShaderBlueprintFunctionLibrary::ObjectPtrs->AddUnique(TSoftObjectPtr<UObject>((*UShaderBlueprintFunctionLibrary::ObjectPaths)[UShaderBlueprintFunctionLibrary::ObjectPaths->Num() - 1]));

			FString relPathGame = TEXT("/Game/") + relPath;
			UShaderBlueprintFunctionLibrary::ObjectPaths->AddUnique(FSoftObjectPath(relPathGame));
			UShaderBlueprintFunctionLibrary::ObjectPtrs->AddUnique(TSoftObjectPtr<UObject>((*UShaderBlueprintFunctionLibrary::ObjectPaths)[UShaderBlueprintFunctionLibrary::ObjectPaths->Num() - 1]));

		}


		if (UShaderBlueprintFunctionLibrary::ObjectPaths->Num() > 0) {
			FStreamableManager& streamableMgr = UAssetManager::GetStreamableManager();
			streamableHandlePtr = streamableMgr.RequestAsyncLoad(*UShaderBlueprintFunctionLibrary::ObjectPaths, FStreamableDelegate::CreateUObject(callback, &UAssetLoadedCallback::OnCreateAllChildren));


			auto loadObj = streamableMgr.LoadSynchronous<UObject>((*UShaderBlueprintFunctionLibrary::ObjectPaths)[1]);
			if (loadObj != nullptr) {
				UE_LOG(MyLog, Log, TEXT("$$ load obj[0] %s "), *(loadObj->GetFName().ToString()));
			}
			else {
				UE_LOG(MyLog, Log, TEXT("$$ load obj[0] failed."), "");
			}

		}
		/*
		TArray<FString> paths;
		paths.Add("/Hotta/Content/Resources/CoreMaterials/MasterMaterials/MM_Unique_spline");
		paths.Add("Hotta/Content/Resources/CoreMaterials/MasterMaterials/MM_Unique_spline");
		paths.Add("/Content/Resources/CoreMaterials/MasterMaterials/MM_Unique_spline");
		paths.Add("Content/Resources/CoreMaterials/MasterMaterials/MM_Unique_spline");
		paths.Add("/Resources/CoreMaterials/MasterMaterials/MM_Unique_spline");
		paths.Add("Resources/CoreMaterials/MasterMaterials/MM_Unique_spline");

		UMaterial* testMat = nullptr;
		for (auto path : paths) {
			FPakEntry entry;
			if (FPakFile::EFindResult::Found == pak->Find(path, &entry)) {
				UE_LOG(MyLog, Log, TEXT("$$ find file:%s"), *path);
			}
			
			testMat = LoadObject<UMaterial>(nullptr, *path);
			if (testMat == nullptr) {
				UE_LOG(MyLog, Log, TEXT("$$ material is null   %s"), *path);
				path.Append(".uasset");
				
				testMat = LoadObject<UMaterial>(nullptr, *path);
				if (testMat == nullptr) {
					UE_LOG(MyLog, Log, TEXT("$$ material is null   %s"), *path);
				}
				else {
					UE_LOG(MyLog, Log, TEXT("$$ material is loaded   %s"), *path);
				}
			}
			else {
				UE_LOG(MyLog, Log, TEXT("$$ material is loaded   %s"), *path);
			}
		}


		if (testMat != nullptr) {
			UE_LOG(MyLog, Log, TEXT("$$ material loaded."), "");
			auto matRes = testMat->GetMaterialResource(ERHIFeatureLevel::ES3_1);
			//FVertexFactoryType vertexFactoryType();
			//auto shader = matRes->GetShader(&vertexFactoryType, 0, true);
			//auto meta = shader->GetRootParametersMetadata()->GetNameStructMap().Find(FHashedName(TEXT("")));
			
		}
		*/
	}

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
	/*
	bool rootFolder = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), TEXT(""));
	bool bFileFolder = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), TEXT("firstPerson"));
	bool bContentFolder = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), FString(TEXT("Game")).Append("/").Append(TEXT("Content")));
	
	UE_LOG(MyLog, Log, TEXT("%s"), (rootFolder ? TEXT("$$ load hotta shader from rootFolder") : TEXT("$$ couldn't load hotta shader from rootFolder")));
	UE_LOG(MyLog, Log, TEXT("%s"), (bFileFolder ? TEXT("$$ load hotta shader from FileFolder") : TEXT("$$ couldn't load hotta shader from FileFolder")));
	UE_LOG(MyLog, Log, TEXT("%s"), (bContentFolder ? TEXT("$$ load hotta shader from ContentFolder") : TEXT("$$ couldn't load hotta shader from ContentFolder")));
	*/
	GetAllVirtualShaderSourcePaths(jsonSeriArr, EShaderPlatform::SP_OPENGL_PCES3_1);
	UE_LOG(MyLog, Log, TEXT("$$ shader source path count: %i"), jsonSeriArr.Num());


	if (GEngine != NULL) {
		//auto pak = LoadPackage(NULL, TEXT(""), ELoadFlags::LOAD_None);
		//if (pak != nullptr) {
		//	UE_LOG(MyLog, Log, TEXT("$$ get asset bundle.."), TEXT("")); 
		//}
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

	auto projConDir = FPaths::ProjectContentDir();
	UE_LOG(MyLog, Log, TEXT("$$ ProjectContentDir:%s"), *projConDir);
	auto shaderWorkDir = FPaths::ShaderWorkingDir();
	UE_LOG(MyLog, Log, TEXT("$$ ShaderWorkingDir:%s"), *shaderWorkDir);
	/*
	auto mat = LoadObject<UObject>(nullptr, TEXT("Content/Resources/CoreMaterials/MasterMaterials/MM_Unique_spline"));
	if (mat == nullptr) {
		UE_LOG(MyLog, Log, TEXT("$$ load mat from content failed"), "");
		mat = LoadObject<UObject>(nullptr, TEXT("/Hotta/Content/Resources/CoreMaterials/MasterMaterials/MM_Unique_spline"));
		if (mat == nullptr) {
			UE_LOG(MyLog, Log, TEXT("$$ load mat from game failed"), "");
		}
		else {
			UE_LOG(MyLog, Log, TEXT("$$ load mat from  game success"), "");
		}
	}
	else {
		UE_LOG(MyLog, Log, TEXT("$$ load mat from content success "), "");
	}
	*/


	FJsonSerializableArray mountedPak;
	pakPlatformFile->GetMountedPakFilenames(mountedPak);
	UE_LOG(MyLog, Log, TEXT("$$ print mounted pak"), "");
	for (auto begin = mountedPak.begin(), end = mountedPak.end(); begin != end; ++begin) {
		UE_LOG(MyLog, Log, TEXT("$$ mounted:%s"), *(*begin));
	}
}


void test() {
	
}