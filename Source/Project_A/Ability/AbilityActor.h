#pragma once
#include "CoreMinimal.h"
#include "AbilityProduct.h"
#include "../Misc/IntervalTicker.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityActor.generated.h"

class UAbility;
class ACharacter;
class UEffectHandler;
class AUnitBase;

UCLASS()
class PROJECT_A_API AAbilityActor : public AAbilityProduct, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	AAbilityActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Duration = 0.f;	// 0 = lasts infinite
	float DurationTimer = 0.f;

	UPROPERTY(EditAnywhere)
	FIntervalTicker Ticker;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Magnitude = 0.f;

	// Reports OnEnd, then defers to the shared AAbilityProduct::Finish() for the generic
	// finish-report/broadcast/destroy logic.
	virtual FVector Finish() override;
};
