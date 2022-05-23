// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeepWiSimGameMode.h"
#include "DeepWiSimCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADeepWiSimGameMode::ADeepWiSimGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/Player/CH_Player"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
