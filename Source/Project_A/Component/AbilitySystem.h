#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem.generated.h"

class UAbilitySlot;
class UOverTimeEffectSlot;
class AUnitBase;
class UUnitDataBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_A_API UAbilitySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAbilitySystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY()
    AUnitBase* MyOwner;

public:
    // -- Config --

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    TArray<UAbilitySlot*> GrantedAbilities;

    // Over-time effects this unit's abilities can apply (see AAbility::ApplyOverTimeEffect). Kept
    // apart from GrantedAbilities, which is indexed by input slot.
    UPROPERTY(BlueprintReadOnly)
    TArray<UOverTimeEffectSlot*> GrantedOverTimeEffects;

    // -- State --

    UPROPERTY(BlueprintReadOnly)
    UAbilitySlot* ActiveAbility = nullptr;

    // -- Functions --

    void InstantiateAbilities(const UUnitDataBase* UnitData);

    // Builds one UOverTimeEffectSlot per entry in UnitData's DefaultOverTimeEffects.
    void InstantiateOverTimeEffects(const UUnitDataBase* UnitData);

    UAbilitySlot* InitiateAbility(int32 Slot);

    UFUNCTION(BlueprintCallable)
    void SetActiveAbility(UAbilitySlot* NewActiveAbility);

    UFUNCTION(BlueprintCallable)
    void EndActiveAbility();

    // Returns the granted slot whose AbilityTag matches exactly (e.g. "Ability.Fireball"), or
    // nullptr with an error log.
    UFUNCTION(BlueprintCallable, Category = "Ability")
    UAbilitySlot* FindSlot(UPARAM(meta = (Categories = "Ability")) FGameplayTag AbilityTag) const;

    // Returns the granted over-time effect slot whose EffectTag matches exactly (e.g.
    // "Effect.Burn"), or nullptr with an error log.
    UFUNCTION(BlueprintCallable, Category = "Effect")
    UOverTimeEffectSlot* FindEffectSlot(UPARAM(meta = (Categories = "Effect")) FGameplayTag EffectTag) const;

private:
    bool IsOwnerDead() const;
};
