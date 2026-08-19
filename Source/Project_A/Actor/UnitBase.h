#pragma once
#include "CoreMinimal.h"
#include "Gameframework/Character.h"
#include "../DataAsset/UnitDataBase.h"
#include "UnitBase.generated.h"

class UAbilitySystem;
class UEffectHandler;
class UBehaviorTree;
class UStats;

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
	
	
	// System Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAbilitySystem* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UEffectHandler* EffectHandlerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UStats* StatsComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UUnitDataBase* UnitData;

	virtual void Initiate(UUnitDataBase* SpawnData);
};
