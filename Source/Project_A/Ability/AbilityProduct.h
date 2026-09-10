#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "../Misc/GameplayEffect.h"
#include "AbilityEventPayload.h"
#include "AbilityProduct.generated.h"

class UAbility;
class ACharacter;

// ============================================================================
// AAbilityProduct
// Shared base for anything an Ability produces that isn't the ability itself
// (AProjectile, AAbilityActor). Holds the parts that are identical between
// them: caster/ability/effect context, tagged event reporting, and a generic
// "this finished" event. Movement, collision, duration/ticking, and any
// lifecycle interface are left entirely to the concrete subclasses.
// ============================================================================

UCLASS(Abstract)
class PROJECT_A_API AAbilityProduct : public AActor
{
	GENERATED_BODY()

public:
	AAbilityProduct();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* MeshComponent;

	UFUNCTION()
	void SetMyAbility(UAbility* Ability);
	UPROPERTY(BlueprintReadOnly)
	UAbility* MyAbility;

	UFUNCTION()
	void SetMyCaster(ACharacter* Caster);
	UPROPERTY(BlueprintReadOnly)
	ACharacter* MyCaster;

	UPROPERTY(BlueprintReadOnly)
	FGameplayEffect MyEffect;

	// Tag reported through ReportAbilityEvent when this actor finishes (see Finish()).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	FGameplayTag FinishEventTag;

	// Reports a tagged event (with contextual Payload) to this actor's owning Ability,
	// which forwards it up to the caster's PlayerUnit.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void ReportAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload);

	// Per-instance hooks - bind to these (e.g. via "Bind Event to...") on a specific reference
	// to attach extra behavior to just that instance, without going through the tag system.
	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityEventDelegate OnHit;

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityEventDelegate OnFinish;

	// Reports FinishEventTag, broadcasts OnFinish, and destroys this actor. Returns the
	// location it finished at (GetActorLocation() by default). Override to determine that
	// location differently - e.g. AProjectile snaps to an explicit hit location first.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual FVector Finish();

protected:
	UPROPERTY()
	bool bHasFinished = false;
};
