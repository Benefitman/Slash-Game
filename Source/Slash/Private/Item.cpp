// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Slash/DebugMacros.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickupInterface.h"
#include "NiagaraComponent.h"

/**
 * @brief Sets default values for this actor's properties.
 *
 * This function is called during the construction of an AItem instance. It sets up the item's mesh, sphere, and effect components.
 */
AItem::AItem()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	// Create and configure the item's mesh component
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = ItemMesh;
	// Create and configure the item's sphere component
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());
	// Create and configure the item's effect component
	ItemEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	ItemEffect->SetupAttachment(GetRootComponent());

}

/**
 * @brief Called when the game starts or when spawned.
 *
 * This function sets up the sphere component's overlap events.
 */
void AItem::BeginPlay()
{
	Super::BeginPlay();
	// Set up the sphere component's overlap events
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	
}

/**
 * @brief Returns the transformed sine value.
 *
 * This function returns the transformed sine value, which is used for the item's hovering effect.
 *
 * @return The transformed sine value.
 */
float AItem::TransformedSin()
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}

/**
 * @brief Returns the transformed cosine value.
 *
 * This function returns the transformed cosine value, which is used for the item's hovering effect.
 *
 * @return The transformed cosine value.
 */
float AItem::TransformedCos()
{
	return Amplitude * FMath::Cos(RunningTime * TimeConstant);
}

/**
 * @brief Handles the event when the item's sphere component overlaps with another component.
 *
 * This function is triggered when the item's sphere component overlaps with another component. It checks if the other actor implements the IPickupInterface. If it does, it calls the SetOverlappingItem function on the other actor, passing the item as a parameter.
 *
 * @param OverlappedComponent The component that was overlapped.
 * @param OtherActor The other actor involved in the overlap.
 * @param OtherComp The other component involved in the overlap.
 * @param OtherBodyIndex The index of the other body involved in the overlap.
 * @param bFromSweep Whether the overlap is from a sweep.
 * @param SweepResult The sweep result.
 */
void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if the other actor implements the IPickupInterface
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		// If it does, call the SetOverlappingItem function on the other actor
		PickupInterface->SetOverlappingItem(this);
	}
}
/**
 * @brief Handles the event when the item's sphere component ends overlap with another component.
 *
 * This function is triggered when the item's sphere component ends overlap with another component. It checks if the other actor implements the IPickupInterface. If it does, it calls the SetOverlappingItem function on the other actor, passing nullptr as a parameter.
 *
 * @param OverlappedComponent The component that was overlapped.
 * @param OtherActor The other actor involved in the overlap.
 * @param OtherComp The other component involved in the overlap.
 * @param OtherBodyIndex The index of the other body involved in the overlap.
 */
void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if the other actor implements the IPickupInterface
	IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor);
	if (PickupInterface)
	{
		// If it does, call the SetOverlappingItem function on the other actor
		PickupInterface->SetOverlappingItem(nullptr);
	}
}

/**
 * @brief Called every frame.
 *
 * This function is called every frame. If the item's state is EIS_Hovering, it adds an offset to the item's world location to create a hovering effect.
 *
 * @param DeltaTime The time elapsed since the last frame.
 */
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	if (ItemState == EItemState::EIS_Hovering)
	{
	AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
	}	
}


