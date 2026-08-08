#include "PlayerUnit.h"
#include "../Component/AbilitySystem.h"
#include "../Component/EffectHandler.h"
#include "../Component/Stats.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


APlayerUnit::APlayerUnit()
{
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystem>(TEXT("AbilitySystemComponent"));
	EffectHandlerComponent = CreateDefaultSubobject<UEffectHandler>(TEXT("EffectHandlerComponent"));
	StatsComponent = CreateDefaultSubobject<UStats>(TEXT("StatsComponent"));

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
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

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
}

void APlayerUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}