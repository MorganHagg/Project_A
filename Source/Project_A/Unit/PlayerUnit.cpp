#include "PlayerUnit.h"
#include "../Component/AbilitySystem.h"
#include "../Component/AttributeComponent.h"
#include "../Component/EffectHandler.h"
#include "../Misc/AttributeSet.h"
#include "../Component/TalentComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "../DataAsset/PlayerUnitData.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerUnit::APlayerUnit()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	TalentComponent = CreateDefaultSubobject<UTalentComponent>(TEXT("Talent"));
	AdjustCamera();
}

void APlayerUnit::SetupUnit(UUnitDataBase* SpawnData)
{
	Super::SetupUnit(SpawnData);
	if (UPlayerUnitData* PlayerData = Cast<UPlayerUnitData>(SpawnData))
	{
		TalentComponent->AcquiredTalents = PlayerData->Talents;
		TalentComponent->AbilitySystemComponent = AbilitySystemComponent;
	}
	
}

void APlayerUnit::AdjustCamera()
{
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->SetRelativeRotation(FRotator(-70.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 700.f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
}

void APlayerUnit::ReceiveDamage(float Amount)
{
	if (AttributeComponent)
	{
		AttributeComponent->ModifyAttribute(EAttributeType::Health, -Amount);
	}
	DelegateOnReceiveDamage(Amount);
}

void APlayerUnit::ReceiveHeal(float Amount)
{
	if (AttributeComponent)
	{
		AttributeComponent->ModifyAttribute(EAttributeType::Health, Amount);
	}
	DelegateOnReceiveHeal(Amount);
}

void APlayerUnit::DelegateOnReceiveDamage(float Amount)
{
	OnReceiveDamage.Broadcast(Amount);
}

void APlayerUnit::DelegateOnReceiveHeal(float Amount)
{
	OnReceiveHeal.Broadcast(Amount);
}
