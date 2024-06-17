// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Treasure.h"
#include "Characters/SlashCharacter.h"
#include "Interfaces/PickupInterface.h"
#include "Kismet/GameplayStatics.h"

/**
 * @brief Handles the event when the treasure's sphere component overlaps with another component.
 *
 * This function is triggered when the treasure's sphere component overlaps with another component. It checks if the other actor implements the IPickupInterface. If it does, it calls the AddGold function on the other actor, passing the treasure as a parameter.
 *
 * It also checks if the other actor is of type ASlashCharacter. If it is, and if the PickupSound is not null, it plays the PickupSound at the treasure's location and destroys the treasure.
 *
 * @param OverlappedComponent The component that was overlapped.
 * @param OtherActor The other actor involved in the overlap.
 * @param OtherComp The other component involved in the overlap.
 * @param OtherBodyIndex The index of the other body involved in the overlap.
 * @param bFromSweep Whether the overlap is from a sweep.
 * @param SweepResult The sweep result.
 */
void ATreasure::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the other actor implements the IPickupInterface
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		// If it does, call the AddGold function on the other actor
		PickupInterface->AddGold(this);
	}

	// Check if the other actor is of type ASlashCharacter
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (PickupSound)
	{
		// If it is, and if the PickupSound is not null, play the PickupSound at the treasure's location
		UGameplayStatics::PlaySoundAtLocation(
		 this, PickupSound, GetActorLocation());
		// Destroy the treasure
		Destroy();
	}
}