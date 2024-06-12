// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SLASH_API AEnemy : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:

	AEnemy();
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	void DirectionalHitReact(const FVector& ImpactPoint);

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

private:

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attribute;

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;
	/**
	* Animation montages
	* */	
	
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;

	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY()
	double CombatRadius = 1000.f;

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
	
	
protected:
	
	virtual void BeginPlay() override;

	void Die();

	bool InTargetRange(AActor* Target, double Radius);

	void MoveToTarget(AActor* Target);

	AActor* ChoosePatrolTarget();
	
	/**
	* Play montage functions
	* */
	
	void PlayHitReactMontage(const FName& SectionName);

	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;

public:	

	
};
