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
	check(IA_Dodge);
	check(IA_Ability1);
	check(IA_Ability2);

	Input->BindAction(
		IA_Move,
		ETriggerEvent::Triggered,
		this,
		&AControllerBase::Move);

	// Bind ability inputs
	TArray<TPair<UInputAction*, EAbilityInputID>> Bindings = {
		{ IA_Dodge, EAbilityInputID::Ability0 },
		{ IA_Ability1, EAbilityInputID::Ability1 },
		{ IA_Ability2, EAbilityInputID::Ability2 }
	};

	for (const auto& Pair : Bindings)
	{
		if (Pair.Key)
		{
			AbilityInputMap.Add(Pair.Key, Pair.Value);
			Input->BindAction(Pair.Key, ETriggerEvent::Started, this, &AControllerBase::OnAbilityInputPressed);
			Input->BindAction(Pair.Key, ETriggerEvent::Completed, this, &AControllerBase::OnAbilityInputReleased);
		}  
	}
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
	if (!PlayerUnit->AbilitySystemComponent) return;

	if (const UInputAction* Action = Instance.GetSourceAction())
	{
		if (const EAbilityInputID* InputID = AbilityInputMap.Find(Action))
		{
			if (PlayerUnit->AbilitySystemComponent->ActiveAbility)
				PlayerUnit->AbilitySystemComponent->ActiveAbility->DoModify();
			else
				PlayerUnit->AbilitySystemComponent->InitializeAbility(static_cast<int32>(*InputID));
		}
	}
}

void AControllerBase::OnAbilityInputReleased(const FInputActionInstance& Instance)
{
	if (!PlayerUnit->AbilitySystemComponent)
		return;

	if (const UInputAction* Action = Instance.GetSourceAction())
	{
		if (AbilityInputMap.Contains(Action))
		{
			PlayerUnit->AbilitySystemComponent->OnAbilityInputReleased();
		}
	}
}

/*
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
*/
