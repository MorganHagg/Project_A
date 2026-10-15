#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AbilityComponent.generated.h"

class UGrantedAbility;
class UAbilityData;
class AUnitBase;
class UUnitDataBase;

// The ability inputs a unit has (see AControllerBase).
UENUM(BlueprintType)
enum class EAbilityInput : uint8
{
    Primary,
    Secondary
};

// The granted abilities one input casts - built from the unit data's FAbilityInputSet. Null where
// the unit data left an entry empty.
USTRUCT(BlueprintType)
struct FGrantedInputAbilities
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UGrantedAbility> Tap;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UGrantedAbility> Hold;

    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UGrantedAbility> Modify;
};

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

    // Every granted ability - the input abilities and the extras (procs, passives). Found by tag
    // with FindGrantedAbility.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
    TArray<UGrantedAbility*> GrantedAbilities;

    UPROPERTY(BlueprintReadOnly)
    FGrantedInputAbilities PrimaryAbilities;

    UPROPERTY(BlueprintReadOnly)
    FGrantedInputAbilities SecondaryAbilities;

    // -- State --

    UPROPERTY(BlueprintReadOnly)
    UGrantedAbility* ActiveAbility = nullptr;

    // -- Functions --

    // Builds one UGrantedAbility per ability in UnitData: its Primary and Secondary input sets, then
    // its ExtraAbilities.
    void InstantiateAbilities(const UUnitDataBase* UnitData);

    // Applies every Passive granted ability to the owner (UEffectComponent::ApplyPassive). Called by
    // AUnitBase::SetupUnit after attributes are instantiated, so passives modify real starting values.
    void ApplyPassives();

    const FGrantedInputAbilities& GetInputAbilities(EAbilityInput Input) const;

    // Activates GrantedAbility and returns it, or nullptr if it is null or the owner is dead.
    UGrantedAbility* InitiateAbility(UGrantedAbility* GrantedAbility);

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

private:
    // Builds one UGrantedAbility from AbilityData and adds it to GrantedAbilities. Returns nullptr for
    // a null AbilityData. Sets bFoundNamelessAbility if it has no AbilityName (editor only).
    UGrantedAbility* GrantAbility(UAbilityData* AbilityData, bool& bFoundNamelessAbility);

    bool IsOwnerDead() const;
};
