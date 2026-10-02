#include "UnitBase.h"
#include "../Component/AbilitySystem.h"
#include "../Component/EffectHandler.h"
#include "../Component/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
	

AUnitBase::AUnitBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystem>(TEXT("AbilitySystemComponent"));
	EffectHandlerComponent = CreateDefaultSubobject<UEffectHandler>(TEXT("EffectHandlerComponent"));
	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));
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

	AbilitySystemComponent->InstantiateAbilities(SpawnData);
	AttributeComponent->InstantiateAttributes(SpawnData);

	GetMesh()->SetSkeletalMesh(SpawnData->Mesh);
	if (SpawnData->AnimationBlueprint)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(SpawnData->AnimationBlueprint);
	}

	// Sizes the capsule to the mesh's height, then fixes z-location and rotation
	if (SpawnData->Mesh)
	{
		const float MeshHalfHeight = SpawnData->Mesh->GetImportedBounds().BoxExtent.Z * GetMesh()->GetRelativeScale3D().Z;
		// Half-height can't go below the radius, so shrink the radius to fit meshes shorter than the capsule's diameter.
		const float Radius = FMath::Min(GetCapsuleComponent()->GetUnscaledCapsuleRadius(), MeshHalfHeight);
		GetCapsuleComponent()->SetCapsuleSize(Radius, MeshHalfHeight, true);
	}
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()),
		FRotator(0.f, -90.f, 0.f));
}

void AUnitBase::HandleDeath()
{
	// Movement is stopped along with collision - without collision, walking movement would drop
	// through the floor.
	SetActorEnableCollision(false);
	GetCharacterMovement()->DisableMovement();

	AbilitySystemComponent->EndActiveAbility();
}

void AUnitBase::ReceiveDamage(float Amount, EAbilityType AbilityType)
{
	if (!AttributeComponent || AttributeComponent->IsDead())
	{
		return;
	}

	const float MitigatedAmount = MitigateDamage(Amount, AbilityType);
	AttributeComponent->ModifyAttribute(EAttributeType::Health, -MitigatedAmount);
	OnReceiveDamage.Broadcast(MitigatedAmount);
}

void AUnitBase::ReceiveHeal(float Amount)
{
	if (!AttributeComponent || AttributeComponent->IsDead())
	{
		return;
	}

	AttributeComponent->ModifyAttribute(EAttributeType::Health, Amount);
	OnReceiveHeal.Broadcast(Amount);
}

float AUnitBase::MitigateDamage(float Damage, EAbilityType AbilityType) const
{
	float Mitigation;
	switch (AbilityType)
	{
	case EAbilityType::Magic:
		Mitigation = AttributeComponent->GetAttribute(EAttributeType::MagicResist);
		break;
	case EAbilityType::Physical:
		Mitigation = AttributeComponent->GetAttribute(EAttributeType::Armour);
		break;
	case EAbilityType::TrueDamage:
	default:
		return Damage;
	}

	const float MitigationPercent = Mitigation / (Mitigation + MitigationConstant);
	return Damage * (1.f - MitigationPercent);
}
