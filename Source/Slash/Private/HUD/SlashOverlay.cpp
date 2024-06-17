// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

/**
 * @brief Sets the health bar percentage for the overlay.
 *
 * This function checks if the HealthProgressBar is null. If it is not, it sets the percentage of the HealthProgressBar to the specified percent.
 *
 * @param Percent The new health percentage to set.
 */
void USlashOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

/**
 * @brief Sets the stamina bar percentage for the overlay.
 *
 * This function checks if the StaminaProgressBar is null. If it is not, it sets the percentage of the StaminaProgressBar to the specified percent.
 *
 * @param Percent The new stamina percentage to set.
 */
void USlashOverlay::SetStaminaBarPercent(float Percent)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(Percent);
	}
}

/**
 * @brief Sets the gold amount for the overlay.
 *
 * This function checks if the GoldText is null. If it is not, it sets the text of the GoldText to the specified gold amount.
 *
 * @param Gold The new gold amount to set.
 */
void USlashOverlay::SetGold(int32 Gold)
{
	if (GoldText)
	{
		const FString String = FString::Printf(TEXT("%d"), Gold);
		const FText Text = FText::FromString(String);
		GoldText->SetText(Text);
	}
}

/**
 * @brief Sets the souls amount for the overlay.
 *
 * This function sets the text of the SoulsText to the specified souls amount.
 *
 * @param Souls The new souls amount to set.
 */
void USlashOverlay::SetSouls(int32 Souls)
{
	const FString String = FString::Printf(TEXT("%d"), Souls);
	const FText Text = FText::FromString(String);
	SoulsText->SetText(Text);
}