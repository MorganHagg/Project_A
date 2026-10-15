#pragma once
#include "CoreMinimal.h"
#include "Gameframework/Character.h"
#include "../DataAsset/UnitDataBase.h"
#include "../Misc/AttributeEffect.h"
#include "UnitBase.generated.h"

class UAbilityComponent;
class UEffectComponent;
class UBehaviorTree;
class UAttributeComponent;
class UWidgetComponent;

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
	// down collision, movement, the active ability, and over-time effects; subclasses extend it
	// (e.g. stopping AI).
	virtual void HandleDeath();
	
	// System Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAbilityComponent* AbilityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UEffectComponent* EffectComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAttributeComponent* AttributeComponent;

	// Screen-space health bar above the unit. Its widget class comes from UnitData's
	// HealthBarWidgetClass in SetupUnit - no bar is shown if that is empty.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UWidgetComponent* HealthBarComponent;

	// Gap between the top of the capsule and the health bar.
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HealthBarHeightOffset = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UUnitDataBase* UnitData;

	// -- Damage / healing --
	// Every Health change from damage or healing goes through these, for every unit type. Amounts are
	// final - scaling, crit, mitigation and incoming modifiers are calculated by UEffectComponent first.

	// Broadcast with the damage actually taken (after mitigation).
	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnUnitReceiveDamage OnReceiveDamage;

	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnUnitReceiveHeal OnReceiveHeal;

	// Clamps Amount at 0, subtracts it from Health, and broadcasts OnReceiveDamage. Returns the damage
	// taken (0 once dead).
	UFUNCTION(BlueprintCallable, Category = "Unit")
	float ReceiveDamage(float Amount);

	// Clamps Amount at 0, adds it to Health, and broadcasts OnReceiveHeal. Returns the amount healed
	// (0 once dead).
	UFUNCTION(BlueprintCallable, Category = "Unit")
	float ReceiveHeal(float Amount);
};
