#pragma once
#include "CoreMinimal.h"
#include "Gameframework/Character.h"
#include "../DataAsset/UnitDataBase.h"
#include "../Ability/Ability.h"
#include "UnitBase.generated.h"

class UAbilitySystem;
class UEffectHandler;
class UBehaviorTree;
class UAttributeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitReceiveDamage, float, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnitReceiveHeal, float, Amount);

UCLASS()
class PROJECT_A_API AUnitBase : public ACharacter
{
	GENERATED_BODY()

public:
	AUnitBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupUnit(UUnitDataBase* SpawnData);

	// Called once by UAttributeComponent when Health reaches 0, before its OnDeath event. Shuts
	// down collision, movement, and the active ability; subclasses extend it (e.g. stopping AI).
	virtual void HandleDeath();
	
	// System Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAbilitySystem* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UEffectHandler* EffectHandlerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAttributeComponent* AttributeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UUnitDataBase* UnitData;

	// -- Damage / healing --
	// Every Health change from damage or healing goes through these, for every unit type.

	// Broadcast with the damage actually taken (after mitigation).
	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnUnitReceiveDamage OnReceiveDamage;

	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnUnitReceiveHeal OnReceiveHeal;

	// Mitigates Amount by AbilityType (see MitigateDamage), subtracts it from Health, and broadcasts
	// OnReceiveDamage. Ignored once dead.
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void ReceiveDamage(float Amount, EAbilityType AbilityType);

	// Adds Amount to Health and broadcasts OnReceiveHeal. Ignored once dead.
	UFUNCTION(BlueprintCallable, Category = "Unit")
	void ReceiveHeal(float Amount);

private:
	// Returns Damage reduced by this unit's mitigation stat for AbilityType (MagicResist for Magic,
	// Armour for Physical, none for True), using MitigationPercent = Mit / (Mit + MitigationConstant).
	float MitigateDamage(float Damage, EAbilityType AbilityType) const;

	// K in the mitigation formula - the stat value that gives 50% reduction.
	static constexpr float MitigationConstant = 400.f;
};
