// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterTemplatePlayerController.h"

#include "Blueprint/UserWidget.h"


void AShooterTemplatePlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);

	UUserWidget* LoseScreen = CreateWidget(this, LoseScreenClass);
	if (LoseScreen != nullptr)
	{
		LoseScreen->AddToViewport();
	}
	
	GetWorldTimerManager().SetTimer(RestartTimer,this, &APlayerController::RestartLevel,RestartDelay);
}
