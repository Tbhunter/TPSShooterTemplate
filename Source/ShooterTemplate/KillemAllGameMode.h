// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterTemplateGameModeBase.h"
#include "KillemAllGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERTEMPLATE_API AKillemAllGameMode : public AShooterTemplateGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PawnKilled(APawn* PawnKilled) override;
};
