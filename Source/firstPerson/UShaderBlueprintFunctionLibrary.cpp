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
#include "ShaderCodeLibrary.h"
#include "ShaderCodeArchive.h"

DEFINE_LOG_CATEGORY(MyLog);


static FString GetCodeArchiveFilename(const FString& BaseDir, const FString& LibraryName, FName Platform)
{
	return BaseDir / FString::Printf(TEXT("ShaderArchive-%s-"), *LibraryName) + Platform.ToString() + TEXT(".ushaderbytecode");
}

void UAssetLoadedCallback::OnCreateAllChildren() {
	UE_LOG(MyLog, Log, TEXT("$$ assetLoaded"), "");
	auto objectPaths = UShaderBlueprintFunctionLibrary::GetObjectPaths();
	auto streamableHandle = UShaderBlueprintFunctionLibrary::GetStreamableHandle();

	//auto objectPtrs = UShaderBlueprintFunctionLibrary::GetObjectPtrs();
	TArray<UObject*> loadedObjects;
	streamableHandle->GetLoadedAssets(loadedObjects);
	
	FShaderCodeArchive* shadercodeArchive = nullptr;
	auto DestFilePath = GetCodeArchiveFilename(FPaths::ProjectContentDir(), TEXT("Hotta"), TEXT("GLSL_ES3_1_ANDROID"));
	TUniquePtr<FArchive> shaderArchive(IFileManager::Get().CreateFileReader(*DestFilePath));
	if (shaderArchive) {
		uint32 Version = 0;
		*shaderArchive << Version;
		shadercodeArchive = FShaderCodeArchive::Create(EShaderPlatform::SP_OPENGL_ES3_1_ANDROID, *shaderArchive, DestFilePath, FPaths::ProjectContentDir(), TEXT("Hotta"));
	}

	auto nameStructMap = FShaderParametersMetadata::GetNameStructMap();
	UE_LOG(MyLog, Log, TEXT("$$ getnameStructMap"), "");
	for (auto nameStruct : nameStructMap) {
		UE_LOG(MyLog, Log, TEXT("$$ shaderVariableName:%s"), nameStruct.Value->GetShaderVariableName());
		UE_LOG(MyLog, Log, TEXT("$$ structTypeName:%s"), nameStruct.Value->GetStructTypeName());

		//for (auto mem : nameStruct.Value->GetMembers()) {
		//	UE_LOG(MyLog, Log, TEXT("$$ memberName:%s"), mem.GetName());
		//}
	}

	for (auto objectPtr : loadedObjects) {
		if (objectPtr != nullptr) {
			UE_LOG(MyLog, Log, TEXT("$$ object loaded: %s"), *(objectPtr->GetFName().ToString()));
			if (objectPtr->GetFName() == TEXT("MM_Hotta_ToonUnlit")) {
				UMaterial* mm = reinterpret_cast<UMaterial*>(objectPtr);
				if (mm != nullptr) {  
					for (auto actor : *UShaderBlueprintFunctionLibrary::GetActors()) {
						
						auto staticMeshCom = Cast<UStaticMeshComponent>(actor);
						staticMeshCom->SetMaterial(0, mm);
						staticMeshCom->Activate(true);
						if (staticMeshCom->OverrideMaterials.Num() > 1) {
							staticMeshCom->OverrideMaterials[0] = mm;
						}
						UE_LOG(MyLog, Log, TEXT("$$ set actor %s material"), *actor->GetFName().ToString());
					}

					TArray<FMaterialParameterInfo> paramsInfo;
					TArray<FGuid> paramsID;
					mm->GetAllScalarParameterInfo(paramsInfo, paramsID); 
					int guidIdx = 0;
					for (auto paramInfo : paramsInfo) {
						UE_LOG(MyLog, Log, TEXT("$$ paramName:%s index:%i guid:%s"), *paramInfo.Name.ToString(), paramInfo.Index, *paramsID[guidIdx].ToString());
						guidIdx++;
					}


					//FLocalVertexFactory vertexF(ERHIFeatureLevel::ES3_1, "");
					const FMaterialResource* matRes = mm->GetMaterialResource(ERHIFeatureLevel::ES3_1);
					UE_LOG(MyLog, Log, TEXT("$$ step1"), "");
					auto shaderMap = matRes->GetGameThreadShaderMap();
					if (shaderMap != nullptr) {
						auto shaderID = shaderMap->GetShaderMapId();
						
						UE_LOG(MyLog, Log, TEXT("$$ step2"), "");
						auto shadermapResource = shaderMap->GetResourceChecked();
						UE_LOG(MyLog, Log, TEXT("$$ step3"), "");
						
						//FMaterialShaderType* shaderType = reinterpret_cast<FMaterialShaderType*>(FShaderType::GetShaderTypeByName(TEXT("FNiagaraShader")));
						UE_LOG(MyLog, Log, TEXT("$$ step4"), "");

						auto rootParamMeta = FShader::GetRootParametersMetadata();

							TMap<FHashedName, TShaderRef<FShader>> shaderList;
							UE_LOG(MyLog, Log, TEXT("$$ shaderNum:%i"), shaderMap->GetShaderNum());
							shaderMap->GetShaderList(shaderList);
							for (auto kv : shaderList) {
								//auto shaderName  = kv.Value.GetRHIShaderBase(EShaderFrequency::SF_Vertex)->ShaderName;
								UE_LOG(MyLog, Log, TEXT("$$ shader codeSize:%u"), kv.Value->GetCodeSize());
								//for (auto param : kv.Value->GetRootParametersMetadata()->GetMembers()) {
								//	UE_LOG(MyLog, Log, TEXT("$$ shader param:%s"), param.GetName());
								//}


								auto resIdx = kv.Value->GetResourceIndex();
								UE_LOG(MyLog, Log, TEXT("$$ shader res idx:%i"), resIdx);

								if (shadercodeArchive != nullptr) {
									FGraphEventArray evtArr;
									shadercodeArchive->PreloadShader(resIdx, evtArr);
									int charSize = 0;
									FSHAHash hash;
									auto shaderCode = shadercodeArchive->GetShaderCode(resIdx, charSize, hash);
									auto chardata = (ANSICHAR*)shaderCode.GetData();
									
									
									for (auto param : kv.Value->Bindings.ResourceParameters) {
										UE_LOG(MyLog, Log, TEXT("$$ paramIdx:%i"), param.BaseIndex);
									}

									//auto kvTmp = kv;
									//ENQUEUE_RENDER_COMMAND(TEST)(
									//	[kvTmp, rootParamMeta](FRHICommandListImmediate& RHICmdList)
									//	{
									//		auto uniformBuffParam = kvTmp.Value->GetUniformBufferParameter(rootParamMeta);
									//		UE_LOG(MyLog, Log, TEXT("$$ uniformBuffParam"), "");
									//	});
									
									const uint8* Base = reinterpret_cast<const uint8*>(&kv.Value->Bindings.Parameters);
									for (const FShaderParameterBindings::FParameterStructReference& ParameterBinding : kv.Value->Bindings.ParameterReferences)
									{
										const TRefCountPtr<FRHIUniformBuffer>& ShaderParameterRef = *reinterpret_cast<const TRefCountPtr<FRHIUniformBuffer>*>(Base + ParameterBinding.ByteOffset);
										
									}


									for (auto uniformBuff : kv.Value->ParameterMapInfo.UniformBuffers) {
										UE_LOG(MyLog, Log, TEXT("$$ uniform baseIdx:%i"), uniformBuff.BaseIndex);
										//auto members = kv.Value->FindAutomaticallyBoundUniformBufferStruct(uniformBuff.BaseIndex)->GetMembers();
										//for (auto mem : members) {
										//	UE_LOG(MyLog, Log, TEXT("$$ memberName:%s"), mem.GetName());
										//}
									}

									/*

									
									for (auto unibuff : kv.Value->Bindings.GraphUniformBuffers) {
										UE_LOG(MyLog, Log, TEXT("$$ uniBuff name:%s  idx:%i"), unibuff.GetTypeLayout().Name, unibuff.BufferIndex);
									}
									for (auto param : kv.Value->Bindings.Parameters) {
										UE_LOG(MyLog, Log, TEXT("$$ parameter baseIdx:%i name:%s"), param.BaseIndex, param.GetTypeLayout().Name);
									}
									*/
								

									//auto metadata = kv.Value->GetP
									//auto staticSlotname = metadata->GetStaticSlotName();
									//auto structName = metadata->GetStructTypeName();
									//auto shaderVariableName = metadata->GetShaderVariableName();
									//UE_LOG(MyLog, Log, TEXT("$$ staticSlotname:%s  structTypeName:%s  shaderVariableName:%s"), staticSlotname, structName, shaderVariableName);

									FString shaderCodeStr(charSize, chardata);
									FFileHelper::SaveStringToFile(shaderCodeStr, *(FString(TEXT("/sdcard/")) + hash.ToString() + FString(TEXT(".glsl"))));

									UE_LOG(MyLog, Log, TEXT("$$ shadercode size:%i code:%s"), charSize, *shaderCodeStr);
									
									//auto rhiShader = shadercodeArchive->CreateShader(resIdx);
									//if (rhiShader != nullptr) {
									//	UE_LOG(MyLog, Log, TEXT("$$ rhiShader name:%s"), *rhiShader->ShaderName);
									//}
								}

								
								//auto rhiShader = shadermapResource->GetShader(resIdx);
								//UE_LOG(MyLog, Log, TEXT("$$ shader name;%s"), rhiShader->GetShaderName());
																
								if (!kv.Value->IsFrozen()) {
									auto type = kv.Value->GetTypeUnfrozen();
									if (type != nullptr) {
										UE_LOG(MyLog, Log, TEXT("$$ shader name:%s"), *type->GetFName().ToString());
									}
								}
							}
							
							UE_LOG(MyLog, Log, TEXT("$$ step5"), "");
						
					}
					
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
TArray<UObject*>* UShaderBlueprintFunctionLibrary::Actors = nullptr;
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

TArray<UObject*>* UShaderBlueprintFunctionLibrary::GetActors() {
	return UShaderBlueprintFunctionLibrary::Actors;
}

void UShaderBlueprintFunctionLibrary::PrintShaderPath() {
	if (Actors != nullptr && Actors->Num() > 0) {
		return;
	}

	UE_LOG(MyLog, Log, TEXT("$$ c++ PrintShaderPath"));
	if (callback == nullptr) {
		callback = NewObject <UAssetLoadedCallback>();
		ObjectPaths = new TArray<FSoftObjectPath>();
		ObjectPtrs = new TArray<TSoftObjectPtr<UObject>>();
		Actors = new TArray<UObject*>();
	}

	
	auto world = GEngine->GetCurrentPlayWorld();   
	if (world == nullptr) {
		UE_LOG(MyLog, Log, TEXT("$$GetCurrentPlayWorld world is null ..."), "");
	}
	if (world != nullptr) {
		UE_LOG(MyLog, Log, TEXT("$$ getWorld ..."), "");
		for (TActorIterator<AStaticMeshActor> it(world); it; ++it) {
			UE_LOG(MyLog, Log, TEXT("$$ find object %s ..."), *it->GetFName().ToString());
			if (it->GetFName().ToString().StartsWith(TEXT("Cube"))) {
				auto staticMeshCom = Cast<UStaticMeshComponent>(it->GetComponentByClass(UStaticMeshComponent::StaticClass()));
				UShaderBlueprintFunctionLibrary::Actors->Add(staticMeshCom);
				UE_LOG(MyLog, Log, TEXT("$$ add act:%s"), *it->GetFName().ToString());
			}
		}
	}
	else {
		UE_LOG(MyLog, Log, TEXT("$$ world is null ..."), "");
		return;
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

		auto globalShaderPath = FPaths::RootDir() + TEXT("Hotta/Content/");
		UE_LOG(MyLog, Log, TEXT("$$ load global shader at %s"), *globalShaderPath); 
		auto bOpenShaderLibGlobal = FShaderCodeLibrary::OpenLibrary(TEXT("Global"), globalShaderPath);
		auto bOpenShaderLib0 = FShaderCodeLibrary::OpenLibrary(TEXT("Global"), FPaths::ProjectContentDir());
		auto bOpenShaderLib1 = FShaderCodeLibrary::OpenLibrary(TEXT("Hotta"), FPaths::ProjectContentDir());

		UE_LOG(MyLog, Log, TEXT("$$ bOpenShaderLibGlobal:%s"), (bOpenShaderLibGlobal ? TEXT("success") : TEXT("failed")));
		UE_LOG(MyLog, Log, TEXT("$$ bOpenShaderLib0:%s"), (bOpenShaderLib0 ? TEXT("success") : TEXT("failed")));
		UE_LOG(MyLog, Log, TEXT("$$ bOpenShaderLib1:%s"), (bOpenShaderLib1 ? TEXT("success") : TEXT("failed")));
		
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

		}
 
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
 

	FJsonSerializableArray mountedPak;
	pakPlatformFile->GetMountedPakFilenames(mountedPak);
	UE_LOG(MyLog, Log, TEXT("$$ print mounted pak"), "");
	for (auto begin = mountedPak.begin(), end = mountedPak.end(); begin != end; ++begin) {
		UE_LOG(MyLog, Log, TEXT("$$ mounted:%s"), *(*begin));
	}
}


void test() {
	
}