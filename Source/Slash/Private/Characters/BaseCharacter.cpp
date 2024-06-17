// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Item/Weapon/Weapon.h"
#include "Components/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

// Constructor for the ABaseCharacter class.
// Sets up the character's attributes and collision responses.
ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attribute = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attribute"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called when the game starts or when spawned.
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

}
// Implementation of the GetHit function.
// Called when the character gets hit.
// Plays hit reactions and sounds, and spawns hit particles.
void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{
		DirectionalHitReact(Hitter->GetActorLocation());
	}
	else Die();
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}
// The Attack function
void ABaseCharacter::Attack()
{
}

// The Die function for the character.
// Plays the death animation when called.
void ABaseCharacter::Die()
{
	PlayDeathMontage();
}


void ABaseCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

// This function, DirectionalHitReact, is responsible for playing a hit reaction animation based on the direction of the hit.
// The function takes a parameter, ImpactPoint, which represents the location where the character was hit.
// The function first calculates the forward vector of the character and the vector from the character to the impact point.
// It then calculates the angle between these two vectors using the dot product (which gives the cosine of the angle) and the arccosine function.
// The angle is then converted from radians to degrees.
// The function then determines the direction of the hit based on this angle.
// If the angle is between -45 and 45 degrees, the hit came from the front.
// If the angle is between -135 and -45 degrees, the hit came from the left.
// If the angle is between 45 and 135 degrees, the hit came from the right.
// Otherwise, the hit came from the back.
// The function then plays the appropriate hit reaction animation based on this direction.
void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	// Lower Impact Point to the Enemy's Actor Location Z
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward||ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	// Take the inverse cosine (arc-cosine) of cos(theta) to get theta
	double Theta = FMath::Acos(CosTheta);
	// convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	// if CrossProduct points down, Theta should be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("FromBack");

	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromLeft");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");
	}

	PlayHitReactMontage(Section);
}

// The PlayHitSound function plays a sound at the specified impact point.
void ABaseCharacter::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			HitSound,
			ImpactPoint
		);
	}
}

// The SpawnHitParticles function spawns hit particles at the specified impact point.
void ABaseCharacter::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles && GetWorld())
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

// The HandleDamage function is called when the character takes damage.
void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attribute)
	{
		Attribute->RecieveDamage(DamageAmount);
	}
}

// The PlayMontageSection function plays a specific section of an animation montage.
void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

// The PlayRandomMontageSection function plays a random section of an animation montage.
int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
		const int32 MaxSectionIndex = SectionNames.Num() - 1;
		const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
		PlayMontageSection(Montage, SectionNames[Selection]);
		return Selection;
}

// The PlayAttackMontage function plays a random section of the attack montage.
int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

// The PlayDeathMontage function plays a random section of the death montage.
int32 ABaseCharacter::PlayDeathMontage()
{
	const int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}
	return Selection;
}

// Disables the character's capsule collision.
void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Disables the character's mesh collision.
void ABaseCharacter::DisableMeshCollision()
{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Checks if the character can attack.
bool ABaseCharacter::CanAttack()
{
	return false;
}

// Checks if the character is alive.
bool ABaseCharacter::IsAlive()
{
	return Attribute && Attribute->IsAlive();
}

// Called when the attack animation ends.
void ABaseCharacter::AttackEnd()
{
}


void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// This function, SetWeaponCollisionEnabled, is used to control the collision detection state of the character's equipped weapon.
// It takes a parameter, CollisionEnabled, which is an enumeration type (ECollisionEnabled::Type) that defines the various states of collision detection.
//
// The function performs the following steps:
// 1. It first checks if the character has an equipped weapon and if the weapon has a collision box (EquippedWeapon && EquippedWeapon->GetWeaponBox()).
//    The GetWeaponBox function likely returns a reference to the weapon's collision box, which is used to detect collisions between the weapon and other objects in the game world.
//
// 2. If the character has an equipped weapon with a collision box, it sets the collision enabled state of the weapon's collision box to the value of CollisionEnabled 
//    (EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);). This controls whether the weapon's collision box is active or not, and thus whether the weapon can interact with other objects in the game world.
//
// 3. Finally, it clears the IgnoreActors list of the equipped weapon (EquippedWeapon->IgnoreActors.Empty();). This list likely contains references to actors that the weapon's collision box should ignore, i.e., not detect collisions with. 
//    By clearing this list, the function ensures that the weapon's collision box will detect collisions with all actors in the game world (unless the collision box's collision responses are set to ignore certain actors).
//
// In summary, this function is used to control the collision detection of the character's equipped weapon, allowing the weapon to interact with other objects in the game world.
void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}
