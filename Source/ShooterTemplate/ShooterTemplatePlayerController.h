// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterTemplatePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERTEMPLATE_API AShooterTemplatePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void GameHasEnded(AActor* EndGameFocus, bool bIsWinner) override;
private:

	UPROPERTY(EditAnywhere )
	TSubclassOf<class UUserWidget> LoseScreenClass;
	
	UPROPERTY(EditAnywhere)
	float RestartDelay = 5;

	FTimerHandle RestartTimer;
	
};
