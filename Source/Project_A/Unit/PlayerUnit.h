#pragma once
#include "CoreMinimal.h"
#include "UnitBase.h"
#include "GameplayTagContainer.h"
#include "../Ability/AbilityEventPayload.h"
#include "PlayerUnit.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UTalentComponent;

// Sole delegation point for talents. Abilities report tagged events here (see
// UAbilitySlot::ReportAbilityEvent); every listening talent receives every event
// and filters by its own RequiredTags (see UTalentBase::HandleAbilityEvent).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityEvent, FGameplayTagContainer, EventTags, FAbilityEventPayload, Payload);

UCLASS()
class PROJECT_A_API APlayerUnit : public AUnitBase
{
	GENERATED_BODY()

public:
	APlayerUnit();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UTalentComponent* TalentComponent;

	virtual void SetupUnit(UUnitDataBase* SpawnData) override;


	void AdjustCamera();

	// --------------------------------------------------------------
	// Talent delegation
	// --------------------------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "Talent")
	FOnAbilityEvent OnAbilityEvent;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	void BroadcastAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload);
};