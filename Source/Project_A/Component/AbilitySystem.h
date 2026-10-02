#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem.generated.h"

class UAbilitySlot;
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
    virtual void TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction
) override;

    // -- Config --

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    TArray<UAbilitySlot*> GrantedAbilities;

    // -- State --

    UPROPERTY(BlueprintReadOnly)
    UAbilitySlot* ActiveAbility = nullptr;

    // -- Functions --

    void InstantiateAbilities(const UUnitDataBase* UnitData);

    UAbilitySlot* InitiateAbility(int32 Slot);

    UFUNCTION(BlueprintCallable)
    void SetActiveAbility(UAbilitySlot* NewActiveAbility);

    UFUNCTION(BlueprintCallable)
    void EndActiveAbility();

    // Returns the granted slot whose AbilityTag matches exactly (e.g. "Ability.Fireball"), or
    // nullptr with an error log.
    UFUNCTION(BlueprintCallable, Category = "Ability")
    UAbilitySlot* FindSlot(UPARAM(meta = (Categories = "Ability")) FGameplayTag AbilityTag) const;

private:
    bool IsOwnerDead() const;
};
