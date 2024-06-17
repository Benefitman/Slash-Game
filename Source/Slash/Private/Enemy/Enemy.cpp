// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/ActorComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Characters/SlashCharacter.h"
#include "Item/Weapon/Weapon.h"
#include "Navigation/PathFollowingComponent.h"

/*
 * Constructor for the AEnemy class.
 * This function sets up the mesh collision properties, creates the health bar widget and pawn sensing component, and configures character movement.
 */
AEnemy::AEnemy()
{
 // Set the actor to tick every frame
 PrimaryActorTick.bCanEverTick = true;

 // Set up the mesh collision properties
 GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
 GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
 GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
 GetMesh()->SetGenerateOverlapEvents(true);

 // Create the health bar widget and attach it to the root component
 HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
 HealthBarWidget->SetupAttachment(GetRootComponent());

 // Configure character movement
 GetCharacterMovement()->bOrientRotationToMovement = true;
 bUseControllerRotationPitch = false;
 bUseControllerRotationYaw = false;
 bUseControllerRotationRoll = false;

 // Create the pawn sensing component and configure its properties
 PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
 PawnSensing->SightRadius = 3500.f;
 PawnSensing->SetPeripheralVisionAngle(55.f);
}

/**
 * @brief Called every frame.
 * Checks if the enemy is dead, and if not, checks the enemy's state and performs the appropriate actions.
 */
void AEnemy::Tick(float DeltaTime)
{
 Super::Tick(DeltaTime);
 if (IsDead()) return;
 if (EnemyState > EEnemyState::EES_Patrolling)
 {
  CheckCombatTarget();
 }
 else
 {
  CheckPatrolTarget();
 }
}

/**
 * @brief Called when the enemy takes damage.
 * Handles the damage, sets the combat target to the instigator of the damage, and starts chasing the target.
 * @return The amount of damage taken.
 */
float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
 HandleDamage(DamageAmount);
 CombatTarget = EventInstigator->GetPawn();
 ChaseTarget();
 return DamageAmount;
}

/**
 * @brief Called when the enemy is destroyed.
 * Destroys the weapon equipped by the enemy.
 */
void AEnemy::Destroyed()
{
 if (EquippedWeapon) EquippedWeapon->Destroy();
};

/**
 * @brief Called when the enemy gets hit.
 * Shows the enemy's health bar.
 */
void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
 Super::GetHit_Implementation(ImpactPoint, Hitter);
 ShowHealthBar();
};

/**
 * @brief Called when the game starts or when the actor is spawned.
 * Sets up the pawn sensing component, initializes the enemy, and adds the "Enemy" tag to the actor.
 */
void AEnemy::BeginPlay()
{
 Super::BeginPlay();
 if (PawnSensing) PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
 InitializeEnemy();
 Tags.Add(FName("Enemy"));
}

/**
 * @brief This function is called when the enemy character dies in the game.
 * 
 * The function performs several tasks to handle the enemy's death:
 * 
 * 1. `Super::Die();` - Calls the Die function in the parent class. This could include any common functionality that should be executed whenever any character dies, not just enemies.
 * 
 * 2. `EnemyState = EEnemyState::EES_Dead;` - Sets the enemy's state to 'Dead'. This is likely an enumeration value that is used elsewhere in the code to check the enemy's state and perform certain actions accordingly.
 * 
 * 3. `ClearAttackTimer();` - Clears the attack timer. This is likely a timer that was set when the enemy started an attack, and it's being cleared now because the enemy is dead and can no longer attack.
 * 
 * 4. `HideHealthBar();` - Hides the enemy's health bar. This is likely a UI element that shows the enemy's current health to the player. It's being hidden now because the enemy is dead.
 * 
 * 5. `DisableCapsule();` - Disables the enemy's capsule. This is likely referring to a capsule collision component that is used for things like collision detection and physics simulation. It's being disabled now because the enemy is dead and no longer needs to interact with other objects in the game world.
 * 
 * 6. `SetLifeSpan(DeathLifeSpan);` - Sets the enemy's lifespan to a certain value. This is likely the amount of time that the enemy's dead body will remain in the game world before it's automatically removed to free up resources.
 */
void AEnemy::Die()
{
 Super::Die();
 EnemyState = EEnemyState::EES_Dead;
 ClearAttackTimer();
 HideHealthBar();
 DisableCapsule();
 SetLifeSpan(DeathLifeSpan);
}

/**
 * @brief Called when the enemy attacks.
 * Sets the enemy's state to attacking and plays the attack montage.
 */
void AEnemy::Attack()
{
 EnemyState = EEnemyState::EES_Engaged;
 Super::Attack();
 PlayAttackMontage();
}

/**
 * @brief Checks if the enemy can attack.
 * @return True if the enemy is inside the attack radius, not currently attacking, not engaged, and not dead. False otherwise.
 */
bool AEnemy::CanAttack()
{
 bool bCanAttack = IsInsideAttackRadius() &&
  !IsAttacking() &&
   !IsEngaged() &&
    !IsDead();
 return bCanAttack;
}

/**
 * @brief Handles the damage taken by the enemy.
 * Updates the enemy's health bar widget.
 */
void AEnemy::HandleDamage(float DamageAmount)
{
 Super::HandleDamage(DamageAmount);
 if (Attribute && HealthBarWidget)
 {
  HealthBarWidget->SetHealthPercent(Attribute->GetHealthPercent());
 }
}

/**
 * @brief Called when the enemy's attack ends.
 * Checks the combat target.
 */
void AEnemy::AttackEnd()
{
 EnemyState = EEnemyState::EES_NoState;
 CheckCombatTarget();
}

/**
 * @brief Initializes the enemy.
 * Sets the enemy's controller, moves the enemy to the patrol target, hides the health bar, and spawns the default weapon.
 */
void AEnemy::InitializeEnemy()
{
 EnemyController = Cast<AAIController>(GetController());
 MoveToTarget(PatrolTarget);
 HideHealthBar();
 SpawnDefaultWeapon();
}

/**
 * @brief Spawns the default weapon for the enemy.
 */
void AEnemy::SpawnDefaultWeapon()
{
 UWorld* World = GetWorld();
 if (World && WeaponClass)
 {
  AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
  DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
  EquippedWeapon = DefaultWeapon;
 }
}

/**
 * @brief Checks the combat target.
 * If the enemy is outside the combat radius, loses interest in the target and starts patrolling.
 * If the enemy is outside the attack radius and not currently chasing, starts chasing the target.
 * If the enemy can attack, starts the attack timer.
 */
void AEnemy::CheckCombatTarget()
{
 if (IsOutsideCombatRadius())
 {
  ClearAttackTimer();
  LoseInterest();
  if (!IsEngaged()) StartPatrolling();
 }
 else if (IsOutsideAttackRadius() && !IsChasing())
 {
  ClearAttackTimer();
  if (!IsEngaged()) ChaseTarget();
 }
 else if (CanAttack())
 {
  StartAttackTimer();
 }
}

/**
 * @brief Checks the patrol target.
 * If the enemy is in range of the patrol target, chooses a new patrol target and sets a timer to finish patrolling.
 */
void AEnemy::CheckPatrolTarget()
{
 if (InTargetRange(PatrolTarget, PatrolRadius))
 {
  PatrolTarget = ChoosePatrolTarget();
  const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
  GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, 7.f);
 }
}

/**
 * @brief Called when the patrol timer finishes.
 * Moves the enemy to the patrol target.
 */
void AEnemy::PatrolTimerFinished()
{
 MoveToTarget(PatrolTarget);
}

/**
 * @brief Hides the enemy's health bar.
 */
void AEnemy::HideHealthBar()
{
 if (HealthBarWidget)
 {
  HealthBarWidget->SetVisibility(false);
 }
}

/**
 * @brief Shows the enemy's health bar.
 */
void AEnemy::ShowHealthBar()
{
 if (HealthBarWidget)
 {
  HealthBarWidget->SetVisibility(true);
 }
}

/**
 * @brief Makes the enemy lose interest in the combat target.
 */
void AEnemy::LoseInterest()
{
 CombatTarget = nullptr;
 HideHealthBar();
}

/**
 * @brief Starts the enemy patrolling.
 */
void AEnemy::StartPatrolling()
{
 EnemyState = EEnemyState::EES_Patrolling;
 GetCharacterMovement()->MaxWalkSpeed = 125.f;
 MoveToTarget(PatrolTarget);
}

/**
 * @brief Makes the enemy chase the combat target.
 */
void AEnemy::ChaseTarget()
{
 EnemyState = EEnemyState::EES_Chasing;
 GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
 MoveToTarget(CombatTarget);
}

/**
 * @brief Clears the patrol timer.
 */
void AEnemy::ClearPatrolTimer()
{
 GetWorldTimerManager().ClearTimer(PatrolTimer);
}

/**
 * @brief Starts the attack timer.
 */
void AEnemy::StartAttackTimer()
{
 EnemyState = EEnemyState::EES_Attacking;
 const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
 GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

/**
 * @brief Clears the attack timer.
 */
void AEnemy::ClearAttackTimer()
{
 GetWorldTimerManager().ClearTimer(AttackTimer);
}

/**
 * @brief Moves the enemy to the specified target.
 */
void AEnemy::MoveToTarget(AActor* Target)
{
 if (EnemyController == nullptr || Target == nullptr) return;
 FAIMoveRequest MoveRequest;
 MoveRequest.SetGoalActor(Target);
 MoveRequest.SetAcceptanceRadius(65.f);
 EnemyController->MoveTo(MoveRequest);
}

/**
 * @brief Chooses a new patrol target for the enemy.
 * @return The new patrol target.
 */
AActor* AEnemy::ChoosePatrolTarget()
{
 TArray<AActor*> ValidTargets;
 for (AActor* Target : PatrolTargets)
 {
  if (Target != PatrolTarget)
  {
   ValidTargets.AddUnique(Target);
  }
 }

 const int32 NumPatrolTargets = ValidTargets.Num();
 if (NumPatrolTargets > 0)
 {
  const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
  return ValidTargets[TargetSelection];
 }
 return nullptr;
}

/**
 * @brief Checks if the enemy is outside the combat radius.
 * @return True if the enemy is outside the combat radius. False otherwise.
 */
bool AEnemy::IsOutsideCombatRadius()
{
 return (!InTargetRange(CombatTarget, CombatRadius));
}

/**
 * @brief Checks if the enemy is outside the attack radius.
 * @return True if the enemy is outside the attack radius. False otherwise.
 */
bool AEnemy::IsOutsideAttackRadius()
{
 return !InTargetRange(CombatTarget, AttackRadius);
}

/**
 * @brief Checks if the enemy is inside the attack radius.
 * @return True if the enemy is inside the attack radius. False otherwise.
 */
bool AEnemy::IsInsideAttackRadius()
{
 return InTargetRange(CombatTarget, AttackRadius);
}

/**
 * @brief Checks if the enemy is chasing the combat target.
 * @return True if the enemy is chasing the combat target. False otherwise.
 */
bool AEnemy::IsChasing()
{
 return EnemyState == EEnemyState::EES_Chasing;
}

/**
 * @brief Checks if the enemy is attacking the combat target.
 * @return True if the enemy is attacking the combat target. False otherwise.
 */
bool AEnemy::IsAttacking()
{
 return EnemyState == EEnemyState::EES_Attacking;
}

/**
 * @brief Checks if the enemy is dead.
 * @return True if the enemy is dead. False otherwise.
 */
bool AEnemy::IsDead()
{
 return EnemyState == EEnemyState::EES_Dead;
}

/**
 * @brief Checks if the enemy is engaged with the combat target.
 * @return True if the enemy is engaged with the combat target. False otherwise.
 */
bool AEnemy::IsEngaged()
{
 return EnemyState == EEnemyState::EES_Engaged;
}

/**
 * @brief Checks if the enemy is in range of the specified target.
 * @return True if the enemy is in range of the target. False otherwise.
 */
bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
 if (Target == nullptr) return false;
 const double DistanceToTarget =  (Target->GetActorLocation() - GetActorLocation()).Size();
 return DistanceToTarget <= Radius;
}

/**
 * @brief Called when the enemy sees a pawn.
 * If the enemy should chase the target, sets the combat target to the seen pawn, clears the patrol timer, and starts chasing the target.
 */
void AEnemy::PawnSeen(APawn* SeenPawn)
{
 const bool bShouldChaseTarget = EnemyState != EEnemyState::EES_Dead &&
  EnemyState != EEnemyState::EES_Chasing &&
   EnemyState < EEnemyState::EES_Attacking &&
    SeenPawn->ActorHasTag(FName("Engageable Target"));
 if (bShouldChaseTarget)
 {
  CombatTarget = SeenPawn;
  ClearPatrolTimer();
  ChaseTarget();
 }
}
