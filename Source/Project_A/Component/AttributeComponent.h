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
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<EAttributeType, float> Attributes;

	
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetAttribute(EAttributeType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttribute(EAttributeType Type, float Value);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ModifyAttribute(EAttributeType Type, float Amount);

	UFUNCTION(BlueprintPure, Category = "Attributes")
	bool IsDead() const { return bIsDead; }

	// Broadcast whenever an attribute's value is written (Speed included), with its new value.
	UPROPERTY(BlueprintAssignable, Category = "Attributes")
	FOnAttributeChanged OnAttributeChanged;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Attributes")
	void OnDeath();

private:
	// The single write point for stored (non-Speed) attributes - writes and broadcasts OnAttributeChanged.
	void StoreAttribute(EAttributeType Type, float Value);

	void SetHealthValue(float NewValue);
	UCharacterMovementComponent* GetMovementComponent() const;

	bool bIsDead = false;
};
