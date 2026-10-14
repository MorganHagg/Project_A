#include "ControllerBase.h"
#include "EnhancedInputComponent.h"
#include "../Ability/GrantedAbility.h"
#include "../Unit/PlayerUnit.h"
#include "../Component/AbilityComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

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

    UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(InputComponent);

    check(MappingContext_Control);
    check(IA_Move);
    check(IA_Primary);
    check(IA_Secondary);

    Input->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AControllerBase::Move);

    TArray<TPair<UInputAction*, EAbilityInput>> Bindings = {
        { IA_Primary, EAbilityInput::Primary },
        { IA_Secondary, EAbilityInput::Secondary }
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

bool AControllerBase::ResolveAbilityInput(const FInputActionInstance& Instance, EAbilityInput& OutInput)
{
    if (!PlayerUnit || !PlayerUnit->AbilityComponent)
        return false;

    const UInputAction* Action = Instance.GetSourceAction();
    if (!Action)
        return false;

    const EAbilityInput* Input = AbilityInputMap.Find(Action);
    if (!Input)
        return false;

    OutInput = *Input;
    return true;
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
    EAbilityInput Input;
    if (!ResolveAbilityInput(Instance, Input))
        return;

    if (ActiveHoldInput.IsSet())
    {
        // A hold is already active: this press modifies it, rather than starting its own tap/hold.
        ApplyModify();
        return;
    }

    for (const auto& Pair : PressedInputs)
    {
        if (Pair.Key != Input)
        {
            // Another ability is still pending its own hold threshold: force it active now, then modify it.
            ActivateHold(Pair.Key);
            ApplyModify();
            return;
        }
    }

    FAbilityPressState& State = PressedInputs.Add(Input);
    State.bHoldThresholdMet = false;

    FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AControllerBase::OnHoldThresholdMet, Input);
    GetWorldTimerManager().SetTimer(State.HoldTimerHandle, Delegate, HoldThreshold, false);
}

void AControllerBase::OnHoldThresholdMet(EAbilityInput Input)
{
    ActivateHold(Input);
}

void AControllerBase::ActivateHold(EAbilityInput Input)
{
    if (!PlayerUnit || !PlayerUnit->AbilityComponent)
        return;

    if (FAbilityPressState* State = PressedInputs.Find(Input))
    {
        GetWorldTimerManager().ClearTimer(State->HoldTimerHandle);
        State->bHoldThresholdMet = true;
    }

    UAbilityComponent* AbilityComponent = PlayerUnit->AbilityComponent;
    AbilityComponent->SetActiveAbility(AbilityComponent->InitiateAbility(AbilityComponent->GetInputAbilities(Input).Hold));
    ActiveHoldInput = Input;
}

void AControllerBase::ApplyModify()
{
    UAbilityComponent* AbilityComponent = PlayerUnit->AbilityComponent;

    UGrantedAbility* ModifyAbility = ActiveHoldInput.IsSet() ? AbilityComponent->GetInputAbilities(ActiveHoldInput.GetValue()).Modify.Get() : nullptr;
    if (!ModifyAbility)
    {
        UE_LOG(LogTemp, Error, TEXT("AControllerBase::ApplyModify - the held input has no Modify ability."));
        return;
    }

    // The Modify ability gets the hold's live Ability (if any) before the hold ends, so it can act
    // on it (e.g. push a Firewall).
    AAbility* HeldAbility = AbilityComponent->ActiveAbility ? AbilityComponent->ActiveAbility->GetSpawnedAbility() : nullptr;
    ModifyAbility->ActivateAbility(HeldAbility);
    if (ModifyAbility->bModifyEndsAbility)
    {
        AbilityComponent->EndActiveAbility();
        ActiveHoldInput.Reset();
    }
}

void AControllerBase::OnAbilityInputReleased(const FInputActionInstance& Instance)
{
    EAbilityInput Input;
    if (!ResolveAbilityInput(Instance, Input))
        return;

    FAbilityPressState State;
    if (!PressedInputs.RemoveAndCopyValue(Input, State))
        return; // Not a tracked press (e.g. it was consumed earlier as a Modify trigger).

    GetWorldTimerManager().ClearTimer(State.HoldTimerHandle);

    UAbilityComponent* AbilityComponent = PlayerUnit->AbilityComponent;

    if (State.bHoldThresholdMet)
    {
        if (ActiveHoldInput == Input)
        {
            AbilityComponent->ReleaseActiveAbility();
            ActiveHoldInput.Reset();
        }
        // Else: already ended by a Modify tap while held.
    }
    else
    {
        AbilityComponent->InitiateAbility(AbilityComponent->GetInputAbilities(Input).Tap);
    }
}