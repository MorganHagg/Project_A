#include "Ability.h"

// Engine classes
#include "Gameframework/Character.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"

// Custom classes
#include "Projectile.h"

UAbility::UAbility()
{
	CurrentState = EAbilityState::None;
	AbilityUUID = FGuid::NewGuid().ToString();
}

FString UAbility::GetAbilityUUID()
{
	return AbilityUUID;
}

void UAbility::Initiate(ACharacter* NewCaster)
{
	MyCaster = NewCaster;
	if (!MyCaster)
	{
		EndAbility();
		return;
	}

	World = MyCaster->GetWorld();
	if (World)
	{
		MyController = UGameplayStatics::GetPlayerController(World, 0);
	}

	ActivateAbility(MyCaster);
}

void UAbility::ActivateAbility(ACharacter* NewCaster)
{
	UE_LOG(LogTemp, Log, TEXT("UAbility::ActivateAbility()"));
	MyCaster = NewCaster;

	switch (AbilityType)
	{
	case EAbilityActivationType::Interactive:
		{
			UWorld* MyWorld = GetWorld();
			if (!MyWorld)
			{
				UE_LOG(LogTemp, Error, TEXT("GetWorld() returned null in ActivateAbility"));
				return;
			}

			World = MyWorld;
			PressStartTime = MyWorld->GetTimeSeconds();
			CurrentState = EAbilityState::Pressed;

			if (MyCaster)
			{
				MyWorld->GetTimerManager().SetTimer(
					ThresholdTimerHandle,
					this,
					&UAbility::ThresholdMet,
					ClickDelay,
					false
				);
			}
			break;
		}

	case EAbilityActivationType::Instant:
		{
			OnInstant();
			break;
		}

	case EAbilityActivationType::Passive:
		{
			OnPassive();
			break;
		}

	case EAbilityActivationType::None:
	default:
		{
			UE_LOG(LogTemp, Error, TEXT("%s has no activation type set"), *AbilityName.ToString());
			break;
		}
	}
}

void UAbility::EndAbility()
{
	UWorld* MyWorld = GetWorld();
	if (MyWorld)
	{
		MyWorld->GetTimerManager().ClearTimer(ThresholdTimerHandle);
	}

	if (CurrentState == EAbilityState::Effect3_Modified)
	{
		OnModify();
	}

	if (CurrentState == EAbilityState::Effect2_Charging)
	{
		OnHoldEnd();
	}
	else if (CurrentState == EAbilityState::Pressed)
	{
		OnTap();
	}

	CurrentState = EAbilityState::None;
	MyCaster = nullptr;
	MyTarget = nullptr;
}

EAbilityState UAbility::GetCurrentState()
{
	return CurrentState;
}

void UAbility::ThresholdMet()
{
	CurrentState = EAbilityState::Effect2_Charging;
	OnHold();
}

// ============================================================================
// Effect library
// ============================================================================

void UAbility::RunEffect_Projectile(FLatentActionInfo LatentInfo, UStaticMesh* Mesh, FVector Target, float Speed)
{
	if (!MyCaster || !World)
	{
		return;
	}

	FLatentActionManager& LAM = World->GetLatentActionManager();
	FEffect_ProjectileAction* ProjectileAction = new FEffect_ProjectileAction(LatentInfo);
	LAM.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, ProjectileAction);

	AProjectile* Projectile = World->SpawnActor<AProjectile>(
		AProjectile::StaticClass(),
		MyCaster->GetActorLocation(),
		MyCaster->GetActorRotation()
	);
	Projectile->SetMyAbility(this);
	Projectile->Destination = Target;
	Projectile->Speed = Speed;

	if (Mesh)
	{
		Projectile->MeshComponent->SetStaticMesh(Mesh);
	}

	TWeakObjectPtr<UAbility> WeakThis = this;
	Projectile->OnHit.BindLambda([WeakThis, ProjectileAction](FVector HitLocation)
	{
		if (WeakThis.IsValid())
		{
			ProjectileAction->bComplete = true;
		}
	});
}

void UAbility::RunEffect_Damage(ACharacter* Target, int32 RawDamage, EAbilityType DamageType)
{
	if (Target && Target->Implements<UAbilityDamageable>())
	{
		IAbilityDamageable::Execute_ReceiveDamage(Target, RawDamage, DamageType);
	}
}

void UAbility::RunEffect_Heal(ACharacter* Target, int32 RawHealing)
{
	if (Target && Target->Implements<UAbilityDamageable>())
	{
		IAbilityDamageable::Execute_ReceiveHeal(Target, RawHealing);
	}
}

TArray<ACharacter*> UAbility::RunEffect_AOE(FVector Location, float Radius, ETargetSelection TargetSelection)
{
	TArray<ACharacter*> Targets;
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;

	if (World && World->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (FOverlapResult& Overlap : Overlaps)
		{
			ACharacter* TargetCharacter = Cast<ACharacter>(Overlap.GetActor());
			if (!TargetCharacter)
			{
				continue;
			}

			// TargetSelection needs a Friendly/Hostile check specific to your
			// gameplay framework (team component, faction, etc). Filter here.
			if (TargetSelection == ETargetSelection::All)
			{
				Targets.AddUnique(TargetCharacter);
			}
			// else: plug in your friendly/hostile check, e.g.:
			// bool bIsFriendly = ...;
			// if ((TargetSelection == ETargetSelection::Friendly && bIsFriendly) ||
			//     (TargetSelection == ETargetSelection::Hostile && !bIsFriendly))
			// {
			//     Targets.AddUnique(TargetCharacter);
			// }
		}
	}

	return Targets;
}

void UAbility::RunEffect_ApplyStasis()
{
	// TODO: implement stasis effect
}

AActor* UAbility::RunEffect_SpawnObject(AActor* SpawnActor, FVector SpawnLocation)
{
	if (!World)
	{
		return nullptr;
	}

	FRotator SpawnRotation = SpawnParams.Owner ? SpawnParams.Owner->GetActorRotation() : FRotator::ZeroRotator;

	AActor* SpawnedActor = World->SpawnActor<AActor>(
		SpawnActor ? SpawnActor->GetClass() : AActor::StaticClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	return SpawnedActor;
}