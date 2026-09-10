#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Misc/AttributeSet.h"
#include "AttributeComponent.generated.h"

class UUnitDataBase;
class UCharacterMovementComponent;

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

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Attributes")
	void OnDeath();

private:
	void SetHealthValue(float NewValue);
	UCharacterMovementComponent* GetMovementComponent() const;

	bool bIsDead = false;
};
