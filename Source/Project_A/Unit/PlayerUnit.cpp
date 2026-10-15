#include "PlayerUnit.h"
#include "../Component/AbilityComponent.h"
#include "../Component/AttributeComponent.h"
#include "../Component/EffectComponent.h"
#include "../Misc/AttributeType.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerUnit::APlayerUnit()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	AdjustCamera();
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

void APlayerUnit::BroadcastAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload)
{
	OnAbilityEvent.Broadcast(EventTags, Payload);
}
