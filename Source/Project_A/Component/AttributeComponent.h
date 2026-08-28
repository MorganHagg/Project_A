#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Misc/AttributeSet.h"
#include "AttributeComponent.generated.h"

class UUnitDataBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();
	
	void InstantiateAttributes(const UUnitDataBase* UnitData);
	

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FResource BaseHealth = FResource(100.f);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<float> Attributes;

	
	float GetAttribute(EAttributeType Type) const;
	void SetAttribute(EAttributeType Type, float Value);
	void ModifyAttribute(EAttributeType Type, float Amount);
};
