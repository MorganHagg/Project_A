#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/LatentActionManager.h"
#include "../Misc/IntervalTicker.h"
#include "Ability.generated.h"

// Forward declarations
class ACharacter;
class AProjectile;
class AAbilityActor;
class UStaticMesh;
class UGameplayEffect;

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
class PROJECT_A_API UAbility : public UObject
{
	GENERATED_BODY()

public:
	UAbility();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bModifyEndsAbility = true;
	
	UPROPERTY(BlueprintReadOnly)
	ACharacter* MyCaster;

	UPROPERTY(BlueprintReadOnly)
	ACharacter* MyTarget;

	UPROPERTY(BlueprintReadOnly)
	APlayerController* MyController;

	UWorld* World;
	FActorSpawnParameters SpawnParams;

	FString AbilityUUID;
	FString GetAbilityUUID();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float CoolDown = 0.f;

	UPROPERTY(EditAnywhere)
	FIntervalTicker Ticker;

	// --------------------------------------------------------------
	// Lifecycle
	// --------------------------------------------------------------

	// Activates this ability. Calls into OnActivate (Blueprint-implementable).
	virtual void InitiateAbility(ACharacter* NewCaster);

	

	// Ends this ability. Calls into OnEnd (Blueprint-implementable).
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void EndAbility();

	// --------------------------------------------------------------
	// Blueprint-buildable effect library (project E)
	// --------------------------------------------------------------
	UFUNCTION(BlueprintCallable)
	void RunEffect_Target(TSubclassOf<UGameplayEffect> Effect, ACharacter* Target);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<ACharacter*> RunEffect_AOE(TSubclassOf<UGameplayEffect> Effect, FVector Location, float Radius, ETargetSelection TargetSelection);

	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo"), Category = "Ability")
	void RunEffect_Projectile(FLatentActionInfo LatentInfo, TSubclassOf<UGameplayEffect> Effect, UStaticMesh* Mesh, FVector Target, float Speed,
		int32 PenetrationCount, FVector& OutLocation);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	AAbilityActor* RunEffect_SpawnActor(TSubclassOf<AAbilityActor> NewActor, FTransform Transform);

	void TickAbility(float DeltaTime);

protected:
	virtual FName GetAbilityName() const { return AbilityName; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityName = FName("NO_NAME_ABILITY");

	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnActivate();
	virtual void OnActivate_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnTick();
	virtual void OnTick_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnEnd();
	virtual void OnEnd_Implementation() {}

	UPROPERTY()
	bool bHasEnded = false;	// Small guard against double end

	// TODO: Implement an interface that takes all the OnEnd(), OnTick() etc
};