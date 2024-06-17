// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Weapon/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"

/**
 * @brief Constructor for the AWeapon class.
 *
 * This function sets up the weapon box and trace components.
 */
AWeapon::AWeapon()
{
 // Create and configure the weapon box
 WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
 WeaponBox->SetupAttachment(GetRootComponent());
 WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
 WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

 // Create and configure the box trace start component
 BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
 BoxTraceStart->SetupAttachment(GetRootComponent());

 // Create and configure the box trace end component
 BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
 BoxTraceEnd->SetupAttachment(GetRootComponent());
}

/**
 * @brief Attaches the weapon mesh to a socket.
 *
 * @param InParent The parent component to attach to.
 * @param InSocketName The name of the socket to attach to.
 */
void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
 FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
 ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}

/**
 * @brief Called when the game starts or when the actor is spawned.
 *
 * This function sets up the weapon box overlap event.
 */
void AWeapon::BeginPlay()
{
 Super::BeginPlay();

 WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::AWeapon::OnBoxOverlap);
}

/**
 * @brief Plays the equip sound.
 */
void AWeapon::PlayEquipSound()
{
 if (EquipSound)
 {
  UGameplayStatics::PlaySoundAtLocation(
   this, EquipSound, GetActorLocation()
  );
 }
}

/**
 * @brief Disables the sphere collision.
 */
void AWeapon::DisableSphereCollision()
{
 if (Sphere)
 {
  Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 }
}

/**
 * @brief Deactivates the embers.
 */
void AWeapon::DiactivateEmbers()
{
 if (ItemEffect)
 {
  ItemEffect->Deactivate();
 }
}

/**
 * @brief Equips the weapon.
 *
 * @param InParent The parent component to attach to.
 * @param InSocketName The name of the socket to attach to.
 * @param NewOwner The new owner of the weapon.
 * @param NewInstigator The new instigator of the weapon.
 */
void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
 ItemState = EItemState::EIS_Equipped;
 SetOwner(NewOwner);
 SetInstigator(NewInstigator);
 AttachMeshToSocket(InParent, InSocketName);
 DisableSphereCollision();
 PlayEquipSound();
 DiactivateEmbers();
}

/**
 * @brief Executes the get hit event.
 *
 * @param BoxHit The hit result.
 */
void AWeapon::ExecuteGetHit(FHitResult BoxHit)
{
 IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor());
 if (HitInterface)
 {
  HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
 }
}

/**
 * @brief Checks if the actor is of the same type.
 *
 * @param OtherActor The other actor to check.
 * @return True if the actor is of the same type, false otherwise.
 */
bool AWeapon::ActorIsSameType(AActor* OtherActor)
{
 return GetOwner()->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

/**
 * @brief Called when the weapon box overlaps with another component.
 *
 * @param OverlappedComponent The overlapped component.
 * @param OtherActor The other actor involved in the overlap.
 * @param OtherComp The other component involved in the overlap.
 * @param OtherBodyIndex The index of the other body involved in the overlap.
 * @param bFromSweep Whether the overlap is from a sweep.
 * @param SweepResult The sweep result.
 */
void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
 if (ActorIsSameType(OtherActor)) return;

 FHitResult BoxHit;
 BoxTrace(BoxHit);

 if (BoxHit.GetActor())
 {
  if (ActorIsSameType(BoxHit.GetActor())) return;

  UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
  ExecuteGetHit(BoxHit);
  CreateFields(BoxHit.ImpactPoint);
 }
}

/**
 * @brief Performs a box trace.
 *
 * This function is used to perform a box trace (or box collision check) in the game world. A box trace is a type of collision detection that checks for collisions within a box-shaped volume in the game world.
 *
 * @param BoxHit The hit result. This is an output parameter that will contain the result of the box trace. It will contain information about what was hit, where it was hit, and other related information.
 *
 * The function performs the following steps:
 * 1. It gets the start and end locations for the box trace from the BoxTraceStart and BoxTraceEnd components respectively.
 * 2. It creates an array of actors to ignore during the box trace. This includes the weapon itself and any actors in the IgnoreActors list.
 * 3. It performs the box trace using the UKismetSystemLibrary::BoxTraceSingle function. This function checks for collisions between the start and end locations, ignoring the actors in the ActorsToIgnore list. The result of the box trace is stored in the BoxHit parameter.
 * 4. If an actor was hit during the box trace, it is added to the IgnoreActors list. This ensures that the actor will be ignored in subsequent box traces.
 */
void AWeapon::BoxTrace(FHitResult& BoxHit)
{
 const FVector Start = BoxTraceStart->GetComponentLocation();
 const FVector End = BoxTraceEnd->GetComponentLocation();

 TArray<AActor*> ActorsToIgnore;
 ActorsToIgnore.Add(this);

 for (AActor* Actor : IgnoreActors)
 {
  ActorsToIgnore.AddUnique(Actor);
 }
 UKismetSystemLibrary::BoxTraceSingle(this, Start, End, BoxTraceExtent, BoxTraceStart->GetComponentRotation(), TraceTypeQuery1, false, ActorsToIgnore, bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None, BoxHit, true);
 IgnoreActors.AddUnique(BoxHit.GetActor());
}
