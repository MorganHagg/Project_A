#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/LatentActionManager.h"
#include "Ability.generated.h"

// Forward declarations
class ACharacter;
class AProjectile;
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

	UPROPERTY(BlueprintReadOnly)
	float CoolDown = 0.f;

	// --------------------------------------------------------------
	// Lifecycle
	// --------------------------------------------------------------

	// Sets up caster/world/controller and kicks off ActivateAbility.
	void Initiate(ACharacter* NewCaster);

	// Activates this ability. Calls into OnActivate (Blueprint-implementable).
	virtual void ActivateAbility(ACharacter* NewCaster);

	// Ends this ability. Calls into OnEnd (Blueprint-implementable).
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void EndAbility();

	// --------------------------------------------------------------
	// Blueprint-buildable effect library (project E)
	// --------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void RunEffect_Target(UGameplayEffect* Effect, ACharacter* Target);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	TArray<ACharacter*> RunEffect_AOE(UGameplayEffect* Effect, FVector Location, float Radius, ETargetSelection TargetSelection);

	UFUNCTION(BlueprintCallable, meta = (Latent, LatentInfo = "LatentInfo"), Category = "Ability")
	void RunEffect_Projectile(FLatentActionInfo LatentInfo, UGameplayEffect* Effect, UStaticMesh* Mesh, FVector Target, float Speed,
		int32 PenetrationCount, FVector& OutLocation);

protected:
	virtual FName GetAbilityName() const { return AbilityName; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityName = FName("NO_NAME_ABILITY");

	// --------------------------------------------------------------
	// This ability's own activate/end logic. Each concrete ability
	// (a tap class, a hold class, a modify class, whatever) implements
	// what happens when it activates and when it ends - it has no
	// concept of being "the tap version" or "the hold version" of
	// anything else.
	// --------------------------------------------------------------

	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnActivate();
	virtual void OnActivate_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnEnd();
	virtual void OnEnd_Implementation() {}
};