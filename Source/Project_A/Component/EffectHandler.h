#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EffectHandler.generated.h"

class AUnitBase;
class UGameplayEffect;
class UStats;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UEffectHandler : public UActorComponent
{
	GENERATED_BODY()

public:
	UEffectHandler();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY()
	AUnitBase* MyTarget;
	
	void UpdateEffect();
	void AddEffect(UGameplayEffect* Effect);
	void RemoveEffect(UGameplayEffect* Effect);
	
	UPROPERTY()
	TArray<TObjectPtr<UGameplayEffect>> GameplayEffects;
};
