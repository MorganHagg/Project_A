#include "ControllerBase.h"
#include "EnhancedInputComponent.h"
#include "../Ability/Ability.h"
#include "../Unit/PlayerUnit.h"
#include "../Component/AbilitySystem.h"
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

    // Primary -> slots 0(tap)/1(hold)/2(modify), Secondary -> slots 3/4/5
    TArray<TPair<UInputAction*, int32>> Bindings = {
        { IA_Primary, 0 },
        { IA_Secondary, 3 }
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

bool AControllerBase::ResolveAbilitySlot(const FInputActionInstance& Instance, int32& OutBaseSlot)
{
    if (!PlayerUnit || !PlayerUnit->AbilitySystemComponent)
        return false;

    const UInputAction* Action = Instance.GetSourceAction();
    if (!Action)
        return false;

    const int32* BaseSlot = AbilityInputMap.Find(Action);
    if (!BaseSlot)
        return false;

    OutBaseSlot = *BaseSlot;
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
    int32 BaseSlot;
    if (!ResolveAbilitySlot(Instance, BaseSlot))
        return;

    if (ActiveHoldBaseSlot != -1)
    {
        // A hold is already active: this press modifies it, rather than starting its own tap/hold.
        ApplyModify();
        return;
    }

    for (const auto& Pair : PressedSlots)
    {
        if (Pair.Key != BaseSlot)
        {
            // Another ability is still pending its own hold threshold: force it active now, then modify it.
            ActivateHold(Pair.Key);
            ApplyModify();
            return;
        }
    }

    FAbilityPressState& State = PressedSlots.Add(BaseSlot);
    State.bHoldThresholdMet = false;

    FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AControllerBase::OnHoldThresholdMet, BaseSlot);
    GetWorldTimerManager().SetTimer(State.HoldTimerHandle, Delegate, HoldThreshold, false);
}

void AControllerBase::OnHoldThresholdMet(int32 BaseSlot)
{
    ActivateHold(BaseSlot);
}

void AControllerBase::ActivateHold(int32 BaseSlot)
{
    if (!PlayerUnit || !PlayerUnit->AbilitySystemComponent)
        return;

    if (FAbilityPressState* State = PressedSlots.Find(BaseSlot))
    {
        GetWorldTimerManager().ClearTimer(State->HoldTimerHandle);
        State->bHoldThresholdMet = true;
    }

    UAbilitySystem* AbilitySystem = PlayerUnit->AbilitySystemComponent;
    AbilitySystem->SetActiveAbility(AbilitySystem->InitiateAbility(BaseSlot + 1));
    ActiveHoldBaseSlot = BaseSlot;
}

void AControllerBase::ApplyModify()
{
    UAbilitySystem* AbilitySystem = PlayerUnit->AbilitySystemComponent;

    if (!AbilitySystem->GrantedAbilities.IsValidIndex(ActiveHoldBaseSlot + 2))
    {
        UE_LOG(LogTemp, Error, TEXT("GrantedAbilities has no slot %d"), ActiveHoldBaseSlot + 2);
        return;
    }

    if (UAbility* ModifyAbility = AbilitySystem->GrantedAbilities[ActiveHoldBaseSlot + 2])
    {
        ModifyAbility->ActivateAbility();
        if (ModifyAbility->bModifyEndsAbility)
        {
            AbilitySystem->EndActiveAbility();
            ActiveHoldBaseSlot = -1;
        }
    }
}

void AControllerBase::OnAbilityInputReleased(const FInputActionInstance& Instance)
{
    int32 BaseSlot;
    if (!ResolveAbilitySlot(Instance, BaseSlot))
        return;

    FAbilityPressState State;
    if (!PressedSlots.RemoveAndCopyValue(BaseSlot, State))
        return; // Not a tracked press (e.g. it was consumed earlier as a Modify trigger).

    GetWorldTimerManager().ClearTimer(State.HoldTimerHandle);

    UAbilitySystem* AbilitySystem = PlayerUnit->AbilitySystemComponent;

    if (State.bHoldThresholdMet)
    {
        if (ActiveHoldBaseSlot == BaseSlot)
        {
            AbilitySystem->EndActiveAbility();
            ActiveHoldBaseSlot = -1;
        }
        // Else: already ended by a Modify tap while held.
    }
    else
    {
        AbilitySystem->InitiateAbility(BaseSlot);
    }
}