#include "UnitBase.h"
#include "../Component/AbilityComponent.h"
#include "../Component/EffectComponent.h"
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
	AbilityComponent = CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilitySystemComponent"));
	EffectComponent = CreateDefaultSubobject<UEffectComponent>(TEXT("EffectHandlerComponent"));
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

	AbilityComponent->InstantiateAbilities(SpawnData);
	AttributeComponent->InstantiateAttributes(SpawnData);
	AbilityComponent->ApplyPassives();

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

	AbilityComponent->EndActiveAbility();

	EffectComponent->HandleUnitDeath();

	HealthBarComponent->SetVisibility(false);
}

float AUnitBase::ReceiveDamage(float Amount)
{
	if (!AttributeComponent || AttributeComponent->IsDead())
	{
		return 0.f;
	}

	const float FinalAmount = FMath::Max(Amount, 0.f);
	AttributeComponent->ModifyAttribute(EAttributeType::Health, -FinalAmount);
	OnReceiveDamage.Broadcast(FinalAmount);
	return FinalAmount;
}

float AUnitBase::ReceiveHeal(float Amount)
{
	if (!AttributeComponent || AttributeComponent->IsDead())
	{
		return 0.f;
	}

	const float FinalAmount = FMath::Max(Amount, 0.f);
	AttributeComponent->ModifyAttribute(EAttributeType::Health, FinalAmount);
	OnReceiveHeal.Broadcast(FinalAmount);
	return FinalAmount;
}
