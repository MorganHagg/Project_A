#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Misc/IntervalTicker.h"
#include "../Misc/GameplayEffect.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "AbilityActor.generated.h"

class UAbility;
class ACharacter;
class UEffectHandler;

UCLASS()
class PROJECT_A_API AAbilityActor : public AActor, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	AAbilityActor();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UStaticMeshComponent* MeshComponent;

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

	
	UFUNCTION()
	void SetMyAbility(UAbility* Ability);
	UPROPERTY(VisibleAnywhere)
	UAbility* MyAbility;

	UFUNCTION()
	void SetMyCaster(ACharacter* Caster);
	UPROPERTY(VisibleAnywhere)
	ACharacter* MyCaster;

	UPROPERTY(VisibleAnywhere)
	FGameplayEffect MyEffect;

	UPROPERTY()
	bool bHasEnded = false;	// Small guard against double end
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void EndAbility();
};