#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TalentBase.generated.h"

class UAbility;
class AProjectile;
class AUnitBase;

UCLASS(Blueprintable)
class PROJECT_A_API UTalentBase : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void BindToAbility(UAbility* Ability);

	UFUNCTION(BlueprintCallable)
	void BindToProjectile(AProjectile* Projectile);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSetup();

	UPROPERTY(BlueprintReadWrite)
	UAbility* MyAbility;

	UPROPERTY(BlueprintReadWrite)
	AProjectile* MyProjectile;

	UFUNCTION(BlueprintImplementableEvent)
	void HandleAbilityHit(UAbility* Ability, AUnitBase* HitActor);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleAbilityOverlap(UAbility* Ability, AActor* OverlapActor);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleProjectileHit(AUnitBase* HitUnit);

	UFUNCTION(BlueprintImplementableEvent)
	void HandleProjectileFinish(FVector Location);

};
