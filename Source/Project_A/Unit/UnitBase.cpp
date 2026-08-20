#include "UnitBase.h"
#include "../Component/AbilitySystem.h"
#include "../Component/EffectHandler.h"
#include "../Component/Stats.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"


AUnitBase::AUnitBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystem>(TEXT("AbilitySystemComponent"));
	EffectHandlerComponent = CreateDefaultSubobject<UEffectHandler>(TEXT("EffectHandlerComponent"));
	StatsComponent = CreateDefaultSubobject<UStats>(TEXT("StatsComponent"));
}

void AUnitBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AUnitBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUnitBase::SetupUnit(UUnitDataBase* SpawnData)
{
	UnitData = SpawnData;

	AbilitySystemComponent->InstantiateAbilities(SpawnData->DefaultAbilities);

	GetMesh()->SetSkeletalMesh(SpawnData->Mesh);
	if (SpawnData->AnimationBlueprint)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(SpawnData->AnimationBlueprint);
	}

	// Fixes z-location and rotation
	GetCapsuleComponent()->SetCapsuleHalfHeight(10.f, true);
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
		FRotator(0.f, -90.f, 0.f));
}
