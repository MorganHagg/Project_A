#include "ControllerBase.h"
#include "EnhancedInputComponent.h"
#include "../Ability/Ability.h"
#include "../Actor/PlayerUnit.h"
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

    UAbilitySystem* AbilitySystem = PlayerUnit->AbilitySystemComponent;

    if (AbilitySystem->ActiveAbility)
    {
        UAbility* ModifyAbility = AbilitySystem->InitiateAbility(PressedBaseSlot + 2);
        check(ModifyAbility);
        
        if (ModifyAbility->bModifyEndsAbility)
        {
            UE_LOG(LogTemp, Warning, TEXT("ModifyEnable is true"))
            AbilitySystem->EndActiveAbility();
        }
            

        return;
    }

    PressedBaseSlot = BaseSlot;
    bHoldThresholdMet = false;

    GetWorldTimerManager().SetTimer(
        HoldTimerHandle, this, &AControllerBase::OnHoldThresholdMet, HoldThreshold, false);
}

void AControllerBase::OnHoldThresholdMet()
{
    bHoldThresholdMet = true;

    if (!PlayerUnit || !PlayerUnit->AbilitySystemComponent)
        return;

    UAbilitySystem* AbilitySystem = PlayerUnit->AbilitySystemComponent;

    if (AbilitySystem)
    {
        UAbility* HoldAbility = AbilitySystem->InitiateAbility(PressedBaseSlot + 1);
        AbilitySystem->SetActiveAbility(HoldAbility);
    }
}

void AControllerBase::OnAbilityInputReleased(const FInputActionInstance& Instance)
{
    int32 BaseSlot;
    if (!ResolveAbilitySlot(Instance, BaseSlot) || BaseSlot != PressedBaseSlot)
        return;
    else
        UE_LOG(LogTemp, Warning, TEXT("Released"))
    
    GetWorldTimerManager().ClearTimer(HoldTimerHandle);

    UAbilitySystem* AbilitySystem = PlayerUnit->AbilitySystemComponent;

    if (bHoldThresholdMet)
    {
        AbilitySystem->EndActiveAbility();
    }
    else
    {
        AbilitySystem->InitiateAbility(PressedBaseSlot);
    }

    PressedBaseSlot = -1;
    bHoldThresholdMet = false;
}