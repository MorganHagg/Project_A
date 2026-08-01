#include "ControllerBase.h"
#include "EnhancedInputComponent.h"
#include "../Actor/PlayerUnit.h"
#include "../Component/AbilitySystem.h"
#include "../Ability/Ability.h"

AControllerBase::AControllerBase()
{
}

void AControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(MappingContext_Control, 0);
	}
	bShowMouseCursor = true;
}

void AControllerBase::PossessPlayerUnit(APlayerUnit* NewPlayerUnit)
{
	PlayerUnit = NewPlayerUnit;
	Possess(PlayerUnit);
}

void AControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (PlayerUnit)
		FaceMouseCursor();
}

void AControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* Input =
		CastChecked<UEnhancedInputComponent>(InputComponent);

	check(MappingContext_Control);
	check(IA_Move);
	check(IA_Ability);

	Input->BindAction(
		IA_Move,
		ETriggerEvent::Triggered,
		this,
		&AControllerBase::Move);

	Input->BindAction(
		IA_Ability,
		ETriggerEvent::Started,
		this,
		&AControllerBase::OnAbilityInputPressed);

	Input->BindAction(
		IA_Ability,
		ETriggerEvent::Completed,
		this,
		&AControllerBase::OnAbilityInputReleased);
}

void AControllerBase::Move(const FInputActionValue& Value)
{
	if (!PlayerUnit)
		return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	PlayerUnit->AddMovementInput(ForwardDirection, MovementVector.Y);
	PlayerUnit->AddMovementInput(RightDirection, MovementVector.X);
}

void AControllerBase::FaceMouseCursor()
{
	if (!PlayerUnit)
		return;
	
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		FVector CursorLocation = HitResult.Location;
		FVector PlayerLocation = PlayerUnit->GetActorLocation();

		FVector Direction = (CursorLocation - PlayerLocation);
		Direction.Z = 0.f;

		FRotator NewRotation = Direction.Rotation();
		PlayerUnit->SetActorRotation(NewRotation);
	}
}

void AControllerBase::OnAbilityInputPressed(const FInputActionInstance& Instance)
{
	if (!PlayerUnit)
		return;
	
	const int32 AbilityIndex = static_cast<int32>(Instance.GetValue().Get<float>());

	PlayerUnit->AbilitySystemComponent->InitializeAbility(AbilityIndex);
}

void AControllerBase::OnAbilityInputReleased(const FInputActionInstance& Instance)
{
	if (!PlayerUnit)
		return;
	
	const int32 AbilityIndex = static_cast<int32>(Instance.GetValue().Get<float>());

	PlayerUnit->AbilitySystemComponent->OnAbilityInputReleased();
}
