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
enum class EAbilityState : uint8
{
	None,
	Pressed,
	Effect2_Charging,
	Effect3_Modified
};

UENUM(BlueprintType)
enum class EAbilityActivationType : uint8
{
	None,			// Stale-protection
	Interactive,	// Tap / Hold / Hold+Modify / Hold+Release (this file's state machine)
	Instant,		// Single immediate effect
	Passive			// Always active, no input needed
};


UENUM(BlueprintType)
enum class ETargetSelection : uint8
{
	Friendly,
	Hostile,
	All
};
 
// ============================================================================
// Interface for characters that can receive ability effects.
// Implement this on your ACharacter subclass so RunEffect_Damage / RunEffect_Heal
// have something to call. Replace with your existing damage interface if you
// already have one.
// ============================================================================


// REMOVE??
/*UINTERFACE(BlueprintType)
class PROJECT_A_API UAbilityDamageable : public UInterface
{
	GENERATED_BODY()
};
 
class PROJECT_A_API IAbilityDamageable
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void ReceiveDamage(int32 RawDamage, EAbilityType DamageType);
 
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void ReceiveHeal(int32 RawHealing);
};*/
 
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
 
	// Called once, when the projectile stops (hit something or reached its destination).
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
 
	// Activating all abilities - dispatches by AbilityType.
	void ActivateAbility(ACharacter* NewCaster);
 
	// On right-click while holding.
	void DoModify() { CurrentState = EAbilityState::Effect3_Modified; };
 
	// Ending all abilities.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void EndAbility();
 
	EAbilityState GetCurrentState();
 
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
	
	// Uncertain if I need
	/*UFUNCTION(BlueprintCallable, Category = "Ability")
	AActor* RunEffect_SpawnObject(AActor* SpawnActor, FVector SpawnLocation);*/
 
protected:
	UPROPERTY(BlueprintReadOnly)
	EAbilityState CurrentState = EAbilityState::None;
 
	// Set per-ability in the blueprint's class defaults - no C++ subclass required.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	EAbilityActivationType AbilityType = EAbilityActivationType::None;

	virtual FName GetAbilityName() const { return AbilityName; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FName AbilityName = FName("NO_NAME_ABILITY");
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	float ClickDelay = 0.3f;
 
	FTimerHandle InputTimerHandle;
	FTimerHandle ThresholdTimerHandle;
	float PressStartTime = 0.f;
 
	// --------------------------------------------------------------
	// The 4 blueprint pins: Tap / Hold / Hold+Modify / Hold+Release
	// --------------------------------------------------------------
 
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnTap();
	virtual void OnTap_Implementation() {}
 
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnHold();
	virtual void OnHold_Implementation() {}
 
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnModify();
	virtual void OnModify_Implementation() {}
 
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnHoldEnd();
	virtual void OnHoldEnd_Implementation() {}
 
	// Passive activation pins.
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void OnPassive();
	virtual void OnPassive_Implementation() {}
 
	void ThresholdMet();
};