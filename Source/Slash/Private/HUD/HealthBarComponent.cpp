// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/HealthBarComponent.h"
#include "Components/ProgressBar.h"
#include "HUD/HealthBar.h"

/**
 * @brief Sets the health percentage for the health bar widget.
 * 
 * This function checks if the HealthBarWidget is null. If it is, it attempts to cast the user widget object to a UHealthBar.
 * If the HealthBarWidget is not null and the HealthBar is not null, it sets the percentage of the HealthBar to the specified percent.
 * 
 */
void UHealthBarComponent::SetHealthPercent(float Percent)
{
	// Check if the HealthBarWidget is null
	if (HealthBarWidget == nullptr)
	{
		// Attempt to cast the user widget object to a UHealthBar
		HealthBarWidget = Cast<UHealthBar>(GetUserWidgetObject());
	}

	// If the HealthBarWidget is not null and the HealthBar is not null
	if (HealthBarWidget && HealthBarWidget->HealthBar)
	{
		// Set the percentage of the HealthBar to the specified percent
		HealthBarWidget->HealthBar->SetPercent(Percent);
	}
}
