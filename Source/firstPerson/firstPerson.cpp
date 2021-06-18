// Fill out your copyright notice in the Description page of Project Settings.

#include "firstPerson.h"
#include "Modules/ModuleManager.h"
#include "IPlatformFilePak.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, firstPerson, "firstPerson" );
DEFINE_LOG_CATEGORY(FirstPersonLog);

void firstPerson::StartupModule() {
	FJsonSerializableArray jsonArr;
	
	FPakPlatformFile::GetPakFolders(TEXT(""), jsonArr);

	for (auto begin = jsonArr.begin(), end = jsonArr.end(); begin != end; ++begin) {
		UE_LOG(FirstPersonLog, Log, TEXT("$$ pakFolder:"), *(*begin));
	}

	auto projConDir = FPaths::ProjectContentDir();
	UE_LOG(FirstPersonLog, Log, TEXT("$$ ProjectContentDir:"), *projConDir);
	auto shaderWorkDir = FPaths::ShaderWorkingDir();
	UE_LOG(FirstPersonLog, Log, TEXT("$$ ShaderWorkingDir:"), *shaderWorkDir);

}