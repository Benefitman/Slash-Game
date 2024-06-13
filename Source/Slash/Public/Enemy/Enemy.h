// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:

	AEnemy();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Destroyed() override;
	
private:
	/**
	 * Components
	 */

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY()
	double CombatRadius = 1000.f;

	UPROPERTY()
	double AttackRadius = 150.f;

	/**
	 * Navigation
	 * */

	UPROPERTY()
	class AAIController* EnemyController;

	//Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;
	
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY()
	double PatrolRadius = 20.f;
	
	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMin = 5.f;
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitMax = 10.f;

	/**
	 * AI Behavior
	 * */
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	void ClearPatrolTimer();
	
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsChasing();
	bool IsAttacking();
	bool IsDead();
	bool IsEngaged();

	/**
	 * Combat
	 */

	void StartAttackTimer();
	void ClearAttackTimer();
	
	FTimerHandle AttackTimer;
	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMin = 0.5f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMax = 1.f;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrollingSpeed = 125.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ChasingSpeed = 300.f;

protected:
	
	virtual void BeginPlay() override;
	virtual void Die() override;
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);
	AActor* ChoosePatrolTarget();
	virtual void Attack() override;
	virtual bool CanAttack() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual int32 PlayDeathMontage() override;
	virtual void AttackEnd() override;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 7.f;
	
	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);
	
	void CheckCombatTarget();
	void CheckPatrolTarget();

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;
	
	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
	
};
