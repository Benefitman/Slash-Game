// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AttributeComponent.h"


// Constructor for the UAttributeComponent class.
// Sets the PrimaryComponentTick.bCanEverTick property to true, allowing the component to tick every frame.
UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts or when the component is spawned.
void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Function to handle damage received by the character.
// Decreases the Health attribute by the Damage amount, ensuring it stays within the range of 0 to MaxHealth.
void UAttributeComponent::RecieveDamage(float Damage)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
}

// Function to get the current health percentage of the character.
// Returns the ratio of the current Health to the MaxHealth.
float UAttributeComponent::GetHealthPercent()
{
	return Health / MaxHealth;
}

// Function to check if the character is alive.
// Returns true if the Health attribute is greater than 0, false otherwise.
bool UAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

// Function to add gold to the character's inventory.
// Increases the Gold attribute by the specified NumberOfGold.
void UAttributeComponent::AddGold(int32 NumberOfGold)
{
	Gold += NumberOfGold;
}

// Function to add souls to the character's inventory.
// Increases the Souls attribute by the specified NumberOfSouls.
void UAttributeComponent::AddSouls(int32 NumberOfSouls)
{
	Souls += NumberOfSouls;
}

// Called every frame.
// Can be used to update the state of the component over time.
void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}