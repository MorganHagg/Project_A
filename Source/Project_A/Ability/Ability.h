#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/LatentActionManager.h"
#include "../Misc/IntervalTicker.h"
#include "../Interfaces/AbilityLifecycle.h"
#include "Ability.generated.h"

// Forward declarations
class AUnitBase;
class AAbilityActor;
class AProjectile;
class UStaticMesh;
class UGameplayEffect;

// ============================================================================
// Delegations
// ============================================================================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityHit, UAbility*, Ability, AUnitBase*, HitUnit);


// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class ETargetSelection : uint8
{
	Friendly,
	Hostile,
	All
};

// ============================================================================
// Latent action used by RunEffect_Projectile - completes when the projectile
// reports a hit via its OnHit delegate.
// ============================================================================

class FEffect_ProjectileAction : public FPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	bool bComplete = false;
	FVector& OutLocation;

	FEffect_ProjectileAction(const FLatentActionInfo& Info, FVector& InOutLocation)
		: ExecutionFunction(Info.ExecutionFunction)
		, OutputLink(Info.Linkage)
		, CallbackTarget(Info.CallbackTarget)
		, OutLocation(InOutLocation)
	{}

	void Finish(FVector Location)
	{
		OutLocation = Location;
		bComplete = true;
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		Response.FinishAndTriggerIf(bComplete, ExecutionFunction, OutputLink, CallbackTarget);
	}
};

// ============================================================================
// UAbility
// An individual ability. Does not know whether it was activated via tap,
// hold, or modify - that decision is made by the controller, which picks
// which UAbility subclass to activate. This class only knows how to
// activate and end itself.
// ============================================================================

UCLASS(Blueprintable)
class PROJECT_A_API UAbility : public UObject, public IAbilityLifecycle
{
	GENERATED_BODY()

public:
	UAbility();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bModifyEndsAbility = true;
	
	UPROPERTY(BlueprintReadOnly)
	AUnitBase* MyCaster;

	UPROPERTY(BlueprintReadOnly)
	APlayerController* MyController;

	UWorld* World;
	FActorSpawnParameters SpawnParams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoolDown = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Cost = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MagnitudeMultiplier = 1.f;

	// GetterFunctions

	float GetCoolDown();
	float GetCost();
	float GetMagnitude();

	UPROPERTY(EditAnywhere)
	FIntervalTicker Ticker;

	// --------------------------------------------------------------
	// Lifecycle
	// --------------------------------------------------------------

	// Sets up the ability for later use
	void SetupAbility(AUnitBase* NewCaster);
	
	// Activates this ability. Calls into OnActivate (Blueprint-implementable).
	virtual void ActivateAbility();

	// Ends this ability. Calls into OnEnd (Blueprint-implementable).
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void EndAbility();

	// Called if an ability is no longer needed
	void KillAbility();

	// --------------------------------------------------------------
	// Blueprint-buildable effect library
	// --------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	void Execute_Target(TSubclassOf<UGameplayEffect> Effect, AUnitBase* Target);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<ACharacter*> Execute_AOE(TSubclassOf<UGameplayEffect> Effect, FVector Location, float Radius, ETargetSelection TargetSelection);

	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo"), Category = "Ability")
	void Execute_Projectile(FLatentActionInfo LatentInfo, TSubclassOf<UGameplayEffect> Effect, UStaticMesh* Mesh, FVector Target, float Speed,
		int32 PenetrationCount, FVector& OutLocation);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	AAbilityActor* Execute_Summon(TSubclassOf<AAbilityActor> NewActor, FTransform Transform);

	void TickAbility(float DeltaTime);

	virtual FName GetAbilityName() const { return AbilityName; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityName = FName("NO_NAME_ABILITY");
protected:
	UPROPERTY()
	bool bHasEnded = false;	// Small guard against double end

	// TODO: Implement an interface that takes all the OnEnd(), OnTick() etc

public:
	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityHit OnHit;

	void DelegateOnHit(AUnitBase* Target);

	UFUNCTION(BlueprintImplementableEvent)
	void OnAbilityHit(AUnitBase* Target);
};