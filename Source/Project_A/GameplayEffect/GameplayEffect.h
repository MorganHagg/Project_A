#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameplayEffect.generated.h"

UCLASS(Blueprintable, BlueprintType)
class PROJECT_A_API UGameplayEffect : public UObject
{
	GENERATED_BODY()
public:

	virtual FName GetEffectName() const { return EffectName; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	FName EffectName = FName("NO_NAME_EFFECT");
	
	float Magnitude = 0;
	float Duration = 0.f;
	float Interval = 0.f;

	void OnStartEffect();	// Runs on application
	void OnTickEffect();	// Runs whenever the EffectHandler's array says it's time to do something
	void OnEndEffect();		// Runs at the end of an effects lifetime. Think Living Bomb /Unstable Affliction
	// Todo: Add logic where how it ends is specified. Timer ran out, mob died, dispelled etc

	UPROPERTY()
	bool bHasEnded = false;
};
