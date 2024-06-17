// Fill out your copyright notice in the Description page of Project Settings.


#include "Soul.h"
#include "Interfaces/PickupInterface.h"

/**
 * @brief Handles the event when the soul's sphere component overlaps with another component.
 *
 * This function is triggered when the soul's sphere component overlaps with another component. It checks if the other actor implements the IPickupInterface. If it does, it calls the AddSouls function on the other actor, passing the soul as a parameter. After this, the soul is destroyed.
 *
 * @param OverlappedComponent The component that was overlapped.
 * @param OtherActor The other actor involved in the overlap.
 * @param OtherComp The other component involved in the overlap.
 * @param OtherBodyIndex The index of the other body involved in the overlap.
 * @param bFromSweep Whether the overlap is from a sweep.
 * @param SweepResult The sweep result.
 */
void ASoul::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the other actor implements the IPickupInterface
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		// If it does, call the AddSouls function on the other actor
		PickupInterface->AddSouls(this);
	}
	// Destroy the soul
	Destroy();
}
