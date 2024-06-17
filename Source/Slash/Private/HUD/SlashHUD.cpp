// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

/**
 * @brief This function is called when the game starts or when the actor is spawned.
 *
 * The function performs several tasks to set up the HUD:
 *
 * 1. Calls the BeginPlay function in the parent class.
 * 2. Gets the world and the first player controller in the world.
 * 3. If the player controller is not null, it creates a widget of type USlashOverlay and adds it to the viewport.
 */
void ASlashHUD::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller)
		{
			SlashOverlay = CreateWidget<USlashOverlay>(Controller, SlashOverlayClass);
			SlashOverlay->AddToViewport();
		}
	}
}
