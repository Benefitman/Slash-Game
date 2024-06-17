// Fill out your copyright notice in the Description page of Project Settings.


#include "Breakable/BreakableActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Item/Treasure.h"
#include "Components/CapsuleComponent.h"

// Constructor for the ABreakableActor class
ABreakableActor::ABreakableActor()
{
	// Disable ticking for this actor, as it doesn't need to update every frame.
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	SetRootComponent(GeometryCollection);
	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GeometryCollection->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}

// Called when the game starts or when spawned
void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();

}

//  Ticking has been disabled (as seen in the constructor)
void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// The GetHit_Implementation function is an implementation of a virtual function GetHit that is likely declared in a parent class or an interface that ABreakableActor is inheriting or implementing.
// This function is called when the ABreakableActor gets hit in the game.
// It first gets a reference to the current world with UWorld* World = GetWorld();. The GetWorld function is a member of AActor and it returns a pointer to the world in which the actor exists.
// It then checks if the world exists and if there are any treasure classes available with if (World && TreasureClasses.Num() > 0). TreasureClasses is likely a member variable of ABreakableActor that holds a list of different types of treasures that can be spawned.
// If the conditions are met, it calculates a location for spawning the treasure.
// It then randomly selects a treasure class from the TreasureClasses array: const int32 Selection = FMath::RandRange(0, TreasureClasses.Num() - 1);.
// Finally, it spawns an instance of the selected treasure class at the calculated location with the same rotation as the actor: World->SpawnActor<ATreasure>(TreasureClasses[Selection], Location, GetActorRotation());.
// In summary, when the ABreakableActor gets hit, it spawns a random treasure from a list of possible treasures at a location slightly above its current location.
void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	UWorld* World = GetWorld();
	if (World && TreasureClasses.Num() > 0)
	{
		FVector Location = GetActorLocation();
		Location.Z += 75.f;
		const int32 Selection = FMath::RandRange(0, TreasureClasses.Num() - 1);
		World->SpawnActor<ATreasure>(TreasureClasses[Selection], Location, GetActorRotation());
	}

}

