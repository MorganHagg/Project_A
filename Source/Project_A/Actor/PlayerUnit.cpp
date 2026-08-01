#include "PlayerUnit.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "../Component/AbilitySystem.h"
#include "../Misc/AttributeSet.h"
#include "../GameplayEffect/GameplayEffect.h"

APlayerUnit::APlayerUnit()
{
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystem>(TEXT("AbilitySystemComponent"));
	Attributes = CreateDefaultSubobject<UAttributeSet>(TEXT("Attributes"));

	//Spring Arm and Camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 700.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void APlayerUnit::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlayerUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}