#pragma once
#include "CoreMinimal.h"
#include "UnitBase.h"
#include "GameplayTagContainer.h"
#include "../Ability/AbilityEventPayload.h"
#include "PlayerUnit.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UTalentComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReceiveDamage, float, Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReceiveHeal, float, Amount);

// Sole delegation point for talents. Abilities report tagged events here (see
// UAbility::ReportAbilityEvent); every listening talent receives every event
// and filters by its own ListenTags (see UTalentBase::HandleAbilityEvent).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityEvent, FGameplayTag, EventTag, FAbilityEventPayload, Payload);

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

	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnPlayerReceiveDamage OnReceiveDamage;

	UPROPERTY(BlueprintAssignable, Category = "Unit")
	FOnPlayerReceiveHeal OnReceiveHeal;

	UFUNCTION(BlueprintCallable, Category = "Unit")
	void ReceiveDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Unit")
	void ReceiveHeal(float Amount);

	void DelegateOnReceiveDamage(float Amount);
	void DelegateOnReceiveHeal(float Amount);

	// --------------------------------------------------------------
	// Talent delegation
	// --------------------------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "Talent")
	FOnAbilityEvent OnAbilityEvent;

	UFUNCTION(BlueprintCallable, Category = "Talent")
	void BroadcastAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload);
};