#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Misc/AttributeSet.h"
#include "AttributeComponent.generated.h"

class UUnitDataBase;
class UCharacterMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChanged, EAttributeType, Attribute, float, NewValue);

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();
	
	void InstantiateAttributes(const UUnitDataBase* UnitData);
	

	// Final values. Every attribute except Health is (Base + Flat) x Multiplier (see BaseAttributes /
	// PercentModifiers); Health is the current value, bounded by MaxHealth. Speed is also pushed to
	// the movement component's MaxWalkSpeed.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EAttributeType, float> Attributes;


	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetAttribute(EAttributeType Type) const;

	// Health: sets the current value. Anything else: sets the base value (Base + Flat), which the
	// attribute's percent multiplier then applies to.
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttribute(EAttributeType Type, float Value);

	// Flat change. Health: changes the current value (can kill). Anything else: changes the base
	// value. MaxHealth changes move Health by the same amount, but never kill (Health stops at 1).
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ModifyAttribute(EAttributeType Type, float Amount);

	// Percent change in whole percentages (10 = +10%, -10 = -10%), added to the attribute's
	// multiplier, so equal and opposite calls cancel exactly. Health and MaxHealth share
	// MaxHealth's multiplier: the bonus (base MaxHealth x percent) is added to both MaxHealth and
	// Health, and never kills (Health stops at 1).
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ModifyAttributePercent(EAttributeType Type, float Percent);

	UFUNCTION(BlueprintPure, Category = "Attributes")
	bool IsDead() const { return bIsDead; }

	// Broadcast whenever an attribute's value is written (Speed included), with its new value.
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnAttributeChanged OnAttributeChanged;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Attributes")
	void OnDeath();

private:
	// The single write point for final values - writes and broadcasts OnAttributeChanged.
	void StoreAttribute(EAttributeType Type, float Value);

	// Recomputes Type's final value from its base and multiplier and stores it. MaxHealth moves
	// Health by the change (non-lethally); Speed is pushed to MaxWalkSpeed. Not for Health.
	void UpdateFinalValue(EAttributeType Type);

	// bCanKill = false stops Health at 1 instead of killing.
	void SetHealthValue(float NewValue, bool bCanKill = true);
	UCharacterMovementComponent* GetMovementComponent() const;

	// Base + Flat per attribute (unused for Health).
	UPROPERTY()
	TMap<EAttributeType, float> BaseAttributes;

	// Percent multiplier per attribute, 1.0 = unmodified (unused for Health - see ModifyAttributePercent).
	UPROPERTY()
	TMap<EAttributeType, float> PercentModifiers;

	bool bIsDead = false;
};
