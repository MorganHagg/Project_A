#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AbilityComponent.generated.h"

class UGrantedAbility;
class UGrantedOverTimeEffect;
class AUnitBase;
class UUnitDataBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UAbilityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAbilityComponent();

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    AUnitBase* MyOwner;

public:
    // -- Config --

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    TArray<UGrantedAbility*> GrantedAbilities;

    // Over-time effects this unit's abilities can apply (see AAbility::ApplyOverTimeEffect). Kept
    // apart from GrantedAbilities, which is indexed by input slot.
    UPROPERTY(BlueprintReadOnly)
    TArray<UGrantedOverTimeEffect*> GrantedOverTimeEffects;

    // -- State --

    UPROPERTY(BlueprintReadOnly)
    UGrantedAbility* ActiveAbility = nullptr;

    // -- Functions --

    void InstantiateAbilities(const UUnitDataBase* UnitData);

    // Builds one UGrantedOverTimeEffect per entry in UnitData's DefaultOverTimeEffects.
    void InstantiateOverTimeEffects(const UUnitDataBase* UnitData);

    UGrantedAbility* InitiateAbility(int32 Slot);

    UFUNCTION(BlueprintCallable)
    void SetActiveAbility(UGrantedAbility* NewActiveAbility);

    UFUNCTION(BlueprintCallable)
    void EndActiveAbility();

    // The hold was released: fires a charging ability (see UGrantedAbility::ReleaseAbility), then ends
    // it. EndActiveAbility ends without firing (Modify, death).
    UFUNCTION(BlueprintCallable)
    void ReleaseActiveAbility();

    // Returns the granted ability whose AbilityTag matches exactly (e.g. "Ability.Fireball"), or
    // nullptr with an error log.
    UFUNCTION(BlueprintCallable, Category = "Ability")
    UGrantedAbility* FindGrantedAbility(UPARAM(meta = (Categories = "Ability")) FGameplayTag AbilityTag) const;

    // Returns the granted over-time effect whose EffectTag matches exactly (e.g.
    // "Effect.Burn"), or nullptr with an error log.
    UFUNCTION(BlueprintCallable, Category = "Effect")
    UGrantedOverTimeEffect* FindGrantedOverTimeEffect(UPARAM(meta = (Categories = "Effect")) FGameplayTag EffectTag) const;

private:
    bool IsOwnerDead() const;
};
