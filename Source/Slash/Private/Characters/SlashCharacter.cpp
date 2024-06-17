// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/SlashCharacter.h"
#include "Animation/AnimMontage.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GroomComponent.h"
#include "Item.h"
#include "Item/Treasure.h"
#include "Soul.h"
#include "Item/Weapon/Weapon.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Components/AttributeComponent.h"

// This constructor sets up the default values for the SlashCharacter class.
ASlashCharacter::ASlashCharacter()
{
	// The PrimaryActorTick.bCanEverTick property is set to false to disable ticking every frame for this actor.
	// Ticking is the process of updating the actor's state for each frame of the game.
	// By setting this property to false, we're saying that this actor doesn't need to update every frame.
	// This can be useful for performance optimization, as it reduces the computational load.
	// In this case, the actor's state doesn't need to be updated every frame, so ticking is disabled.
	PrimaryActorTick.bCanEverTick = false;

	// Disable controller rotation for pitch, yaw, and roll
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Set the character to orient its rotation to its movement direction
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// Set the rotation rate for the character when it is moving
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	// Set the collision object type for the character's mesh
	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	// Set the character's mesh to ignore all collision channels
	GetMesh()->SetCollisionResponseToAllChannels(ECR_Ignore);
	// Set the character's mesh to block the visibility collision channel
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	// Set the character's mesh to overlap with the world dynamic collision channel
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	// Enable overlap events for the character's mesh
	GetMesh()->SetGenerateOverlapEvents(true);
	// Create and attach a spring arm component for the camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	// Set the target arm length for the camera boom
	CameraBoom->TargetArmLength = 270.f;
	
	// Create and attach a camera component
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	// Create and attach a groom component for the character's hair
	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	// Set the attachment name for the hair groom component
	Hair->AttachmentName = FString("head");

	// Create and attach a groom component for the character's eyebrows
	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment((GetMesh()));
	// Set the attachment name for the eyebrows groom component
	Eyebrows->AttachmentName = FString("head");
}

// This function is called when the game starts or when the actor is spawned
void ASlashCharacter::BeginPlay()
{
	// Call the parent class's BeginPlay function
	Super::BeginPlay();

	// If the controller is a player controller, add the SlashContext mapping context to the enhanced input local player subsystem
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}

	// The "Engageable Target" tag is added to the actor's list of tags. 
	// Tags are used to categorize actors and to identify, filter, or find actors with specific characteristics or behaviors.
	// In this case, the "Engageable Target" tag is used to identify actors that can be targeted or engaged with by attacking or interacting.
	Tags.Add(FName("Engageable Target"));

	// The InitializeSlashOverlay function is called to set up the SlashOverlay HUD (Heads-Up Display) element for this character.
	// The HUD is the on-screen display of information and controls that the player sees during gameplay.
	// The SlashOverlay might contain elements like the character's health bar, gold, souls and stamina bar.
	// By calling this function in BeginPlay (which is called when the game starts or when the actor is spawned), we ensure that the HUD is set up right from the start of the game.
	InitializeSlashOverlay();
}

// This function is called to move the character based on input
void ASlashCharacter::Move(const FInputActionValue& Value)
{
	// This line of code is a guard clause that checks the character's current action state.
	// The ActionState is an enumeration (EActionState) that represents the current action the character is performing.
	// EAS_Unoccupied means the character is not currently performing any action and is free to move.
	// If the character's ActionState is not EAS_Unoccupied (i.e., the character is performing some action), the function returns immediately.
	// This prevents the character from moving while performing actions like attacking, equipping a weapon, etc.
	// In other words, this line ensures that the character can only move when it is not busy with other actions.
	if (ActionState != EActionState::EAS_Unoccupied) return;
	// Get the movement vector from the input action value
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// Get the controller's rotation and create a yaw rotation from it
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	// Calculate the forward and right directions based on the yaw rotation
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	// Add movement input in the forward direction
	AddMovementInput(ForwardDirection, MovementVector.Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	// Add movement input in the right direction
	AddMovementInput(RightDirection, MovementVector.X);
}

// This function is called to rotate the character's view based on input
void ASlashCharacter::Look(const FInputActionValue& Value)
{
	// Get the look axis vector from the input action value
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// Add pitch and yaw input to rotate the character's view
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

// This function is called when the E key is pressed
void ASlashCharacter::EKeyPressed()
{
// The OverlappingItem is an item that the character is currently overlapping with, or in other words, is close enough to interact with.
// The Cast<AWeapon>(OverlappingItem) function attempts to cast the OverlappingItem to the AWeapon type.
// This is essentially checking if the OverlappingItem is a weapon.
AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);

// If the cast was successful (i.e., the OverlappingItem is indeed a weapon), the character equips this weapon.
if (OverlappingWeapon)
{
	// The EquipWeapon function is called with the OverlappingWeapon as the argument.
	// This function would handle the logic of equipping the weapon, such as attaching it to the character's hand in the game world.
	EquipWeapon(OverlappingWeapon);
}
else
{
	// If the OverlappingItem is not a weapon, the character's current state is checked to determine the next course of action.
	// If the character can disarm (i.e., it is currently holding a weapon), it starts the process of unequipping the current weapon.
	if (CanDisarm())
	{
		// The PlayEquipMontage function is called with "Unequip" as the argument, which would play the unequip animation.
		PlayEquipMontage(FName("Unequip"));
		// The character's state is then set to ECS_Unequipped, indicating that the character is not currently holding a weapon.
		CharacterState = ECharacterState::ECS_Unequipped;
		// The character's action state is set to EAS_EquippingWeapon, indicating that the character is in the process of changing weapons.
		ActionState = EActionState::EAS_EquippingWeapon;
	}
	// If the character can arm (i.e., it is not currently holding a weapon), it starts the process of equipping a weapon.
	else if (CanArm())
	{
		// The PlayEquipMontage function is called with "Equip" as the argument, which would play the equip animation.
		PlayEquipMontage(FName("Equip"));
		// The character's state is then set to ECS_EquippedOneHandedWeapon, indicating that the character is now holding a one-handed weapon.
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
		// The character's action state is set to EAS_EquippingWeapon, indicating that the character is in the process of changing weapons.
		ActionState = EActionState::EAS_EquippingWeapon;
	}
}
}

// This function is called to attack
void ASlashCharacter::Attack()
{
	// If the character can attack, play the attack montage
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

// This function checks if the character can attack
bool ASlashCharacter::CanAttack()
{
	// The character can attack if it is unoccupied and not unequipped
	return ActionState == EActionState::EAS_Unoccupied &&
	 CharacterState != ECharacterState::ECS_Unequipped;
}

// This function is called to equip a weapon
void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	// Equip the weapon to the character's right hand socket and set the weapon's owner and instigator to this character
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	Weapon->SetOwner(this);
	Weapon->SetInstigator(this);
	// Set the character state to equipped with a one-handed weapon
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	// Clear the overlapping item
	OverlappingItem = nullptr;
	// Set the equipped weapon
	EquippedWeapon = Weapon;
}

// This function is called to play the equip montage
void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	// Get the character's animation instance
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	// If the animation instance and equip montage are valid, play the equip montage and jump to the specified section
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

// This function checks if the character can disarm
bool ASlashCharacter::CanDisarm()
{
	// The character can disarm if it is unoccupied and not unequipped
	return ActionState == EActionState::EAS_Unoccupied &&
	 CharacterState != ECharacterState::ECS_Unequipped;
}

// This function checks if the character can arm
bool ASlashCharacter::CanArm()
{
	// The character can arm if it is unoccupied, unequipped, and has an equipped weapon
	return ActionState == EActionState::EAS_Unoccupied &&
	 CharacterState == ECharacterState::ECS_Unequipped &&
	  EquippedWeapon;
}

// This function is called to disarm the character
void ASlashCharacter::Disarm()
{
	// If the character has an equipped weapon, attach it to the spine socket
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

// This function is called to arm the character
void ASlashCharacter::Arm()
{
	// If the character has an equipped weapon, attach it to the right hand socket
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

// This function is called when the character finishes equipping
void ASlashCharacter::FinishEquipping()
{
	// Set the action state to unoccupied
	ActionState = EActionState::EAS_Unoccupied;
}

// This function is called when the character finishes a hit reaction
void ASlashCharacter::HitReactEnd()
{
	// Set the action state to unoccupied
	ActionState = EActionState::EAS_Unoccupied;
}

// This function is called to dodge
void ASlashCharacter::Dodge()
{
	// TODO:: Implement dodge functionality
}

// This function is called when the character finishes an attack
void ASlashCharacter::AttackEnd()
{
	// Set the action state to unoccupied
	ActionState = EActionState::EAS_Unoccupied;
}

// This function is called when the character dies
void ASlashCharacter::Die()
{
	// Call the parent class's Die function
	Super::Die();
	// Set the action state to dead
	ActionState = EActionState::EAS_Dead;
	// Disable collision for the character's mesh
	DisableMeshCollision();
}

// This function is called to make the character jump
void ASlashCharacter::Jump()
{
	// If the character is not unoccupied, return without jumping
	if (ActionState != EActionState::EAS_Unoccupied) return;
	// Call the parent class's Jump function
	Super::Jump();
}

// This function is called to set the overlapping item
void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	// Set the overlapping item
	OverlappingItem = Item;
}

// This function is called to add souls to the character
void ASlashCharacter::AddSouls(ASoul* Soul)
{
	// If the attribute and SlashOverlay are valid, add the souls to the attribute and update the SlashOverlay
	if (Attribute && SlashOverlay)
	{
		Attribute->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attribute->GetSouls());
	}
}

// This function is called to add gold to the character
void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	// If the attribute and SlashOverlay are valid, add the gold to the attribute and update the SlashOverlay
	if (Attribute && SlashOverlay)
	{
		Attribute->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attribute->GetGold());
	}
}

// This function is called to set up the player input component
void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Call the parent class's SetupPlayerInputComponent function
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// If the input component is an enhanced input component, bind the action functions to the input actions
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
	}

}

// This function is called when the character takes damage
float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
								  AActor* DamageCauser)
{
	// Handle the damage and update the HUD health
	HandleDamage(DamageAmount);
	SetHUDHealth();
	// Return the damage amount
	return DamageAmount;
}

// This function is called when the character gets hit
void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	// Call the parent class's GetHit_Implementation function
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	// Disable collision for the character's weapon
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	// If the attribute is valid and the character's health is above 0, set the action state to hit reaction
	if (Attribute && Attribute->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

// This function is called to initialize the SlashOverlay HUD element
void ASlashCharacter::InitializeSlashOverlay()
{
	// If the controller is a player controller, get the SlashOverlay from the SlashHUD
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			// If the SlashOverlay and attribute are valid, set the SlashOverlay's health, stamina, gold, and souls
			if (SlashOverlay && Attribute)
			{
				SlashOverlay->SetHealthBarPercent(Attribute->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}
}

// This function updates the HUD's health display.
void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attribute)
	{
		SlashOverlay->SetHealthBarPercent(Attribute->GetHealthPercent());
	}
}
