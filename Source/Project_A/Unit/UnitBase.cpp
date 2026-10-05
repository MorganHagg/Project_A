#include "UnitBase.h"
#include "../Component/AbilitySystem.h"
#include "../Component/EffectHandler.h"
#include "../Component/AttributeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "../UI/HealthBarWidget.h"
	

AUnitBase::AUnitBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystem>(TEXT("AbilitySystemComponent"));
	EffectHandlerComponent = CreateDefaultSubobject<UEffectHandler>(TEXT("EffectHandlerComponent"));
	AttributeComponent = CreateDefaultSubobject<UAttributeComponent>(TEXT("AttributeComponent"));

	// Screen space: always faces the camera at a constant size. Drawn at the widget's desired size,
	// so WBP_HealthBar's own layout decides how big the bar is.
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawAtDesiredSize(true);
	HealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarComponent->SetGenerateOverlapEvents(false);
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
	AbilitySystemComponent->InstantiateOverTimeEffects(SpawnData);
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

	// Health bar - placed after the capsule resize above, and after InstantiateAttributes so the
	// bar starts at the unit's real Health / MaxHealth.
	HealthBarComponent->SetRelativeLocation(
		FVector(0.f, 0.f, GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() + HealthBarHeightOffset));
	HealthBarComponent->SetWidgetClass(SpawnData->HealthBarWidgetClass);
	// SetWidgetClass only creates the widget itself once play has begun - InitWidget covers the rest.
	HealthBarComponent->InitWidget();
	if (UHealthBarWidget* HealthBar = Cast<UHealthBarWidget>(HealthBarComponent->GetUserWidgetObject()))
	{
		HealthBar->SetOwnerUnit(this);
	}
}

void AUnitBase::HandleDeath()
{
	// Movement is stopped along with collision - without collision, walking movement would drop
	// through the floor.
	SetActorEnableCollision(false);
	GetCharacterMovement()->DisableMovement();

	AbilitySystemComponent->EndActiveAbility();

	EffectHandlerComponent->HandleUnitDeath();

	HealthBarComponent->SetVisibility(false);
}

void AUnitBase::ReceiveDamage(float Amount, EAbilityType AbilityType)
{
	if (!AttributeComponent || AttributeComponent->IsDead())
	{
		return;
	}

	// Rounded to whole damage so Health stays whole - otherwise mitigation leaves fractional
	// remainders (e.g. 0.4 Health) that look like 0 on the health bar but aren't dead.
	const float MitigatedAmount = FMath::RoundToFloat(MitigateDamage(Amount, AbilityType));
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
