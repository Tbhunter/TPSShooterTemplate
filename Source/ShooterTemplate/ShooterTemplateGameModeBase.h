// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterTemplateGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERTEMPLATE_API AShooterTemplateGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void PawnKilled(APawn* PawnKilled);
};
