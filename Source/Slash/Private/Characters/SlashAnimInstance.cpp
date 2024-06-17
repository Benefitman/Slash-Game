// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashAnimInstance.h"
#include "Characters/SlashCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// This function is called when the animation instance is initialized.
// It sets up the SlashCharacter and SlashCharacterMovement references.
void USlashAnimInstance::NativeInitializeAnimation()
{
	// Call the parent class's implementation of this function
	Super::NativeInitializeAnimation();
	// Try to get the owner of this animation instance and cast it to ASlashCharacter
	SlashCharacter = Cast<ASlashCharacter>(TryGetPawnOwner());
	// If the cast was successful, get the character's movement component
	if (SlashCharacter)
	{
		SlashCharacterMovement = SlashCharacter->GetCharacterMovement();
	}
}

// This function is called every frame to update the animation instance.
// It updates the animation variables based on the character's state.
void USlashAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	// Call the parent class's implementation of this function
	Super::NativeUpdateAnimation(DeltaTime);
	// If the SlashCharacterMovement reference is valid, update the animation variables
	if (SlashCharacterMovement)
	{
		// Update the GroundSpeed variable based on the character's velocity
		GroundSpeed = UKismetMathLibrary::VSizeXY(SlashCharacterMovement->Velocity);
		// Update the IsFalling variable based on the character's falling state
		IsFalling = SlashCharacterMovement->IsFalling();
		// Update the CharacterState variable based on the character's state
		CharacterState = SlashCharacter->GetCharacterState();
		// Update the ActionState variable based on the character's action state
		ActionState = SlashCharacter->GetActionState();
		// Update the DeathPose variable based on the character's death pose
		DeathPose = SlashCharacter->GetDeathPose();
	}
}
