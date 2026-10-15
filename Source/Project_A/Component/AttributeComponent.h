#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Misc/AttributeType.h"
#include "AttributeComponent.generated.h"

class UUnitDataBase;
class UCharacterMovementComponent;
struct FAttributeEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChanged, EAttributeType, Attribute, float, NewValue);

// A temporary stat change owned by Source (e.g. a UOverTimeEffect). It stays out of the base values
// and is folded into the final value until Source removes it.
struct FAttributeModifier
{
	EAttributeType Attribute = EAttributeType::Health;
	bool bPercent = false;
	float Magnitude = 0.f;	// Signed; percents in whole numbers (10 = +10%)
	TWeakObjectPtr<const UObject> Source;
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();
	
	void InstantiateAttributes(const UUnitDataBase* UnitData);
	

	// Final values. Every attribute except Health is (Base + Flat) x Multiplier, counting both the
	// permanent values (BaseAttributes / PercentModifiers) and the active Modifiers; Health is the
	// current value, bounded by MaxHealth. Speed is also pushed to the movement component's MaxWalkSpeed.
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

	// Adds a temporary stat change owned by Source, kept until RemoveModifiers(Source). Add/Subtract
	// are flat, AddPercentage/SubtractPercentage percent; a Health percentage applies to MaxHealth,
	// as in ModifyAttributePercent. Not for flat Health changes (damage/heal).
	void AddModifier(const UObject* Source, const FAttributeEffect& Effect);

	// Removes every modifier Source added and recomputes the attributes they changed.
	void RemoveModifiers(const UObject* Source);

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

	// (Base + Flat) x Multiplier for Type, including active modifiers. Not for Health.
	float ComputeFinalValue(EAttributeType Type) const;

	// Recomputes Type's final value and stores it. MaxHealth moves Health by the change
	// (non-lethally); Speed is pushed to MaxWalkSpeed. Not for Health.
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

	// Active temporary stat changes, in the order they were added.
	TArray<FAttributeModifier> Modifiers;

	bool bIsDead = false;
};
