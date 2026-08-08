#include "Ability.h"
#include "Gameframework/Character.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Projectile.h"
#include "../GameplayEffect/GameplayEffect.h"
#include "../Component/EffectHandler.h"

UAbility::UAbility()
{
	CurrentState = EAbilityState::None;
	AbilityUUID = FGuid::NewGuid().ToString();
	World = GetWorld();
}

FString UAbility::GetAbilityUUID()
{
	return AbilityUUID;
}

void UAbility::Initiate(ACharacter* NewCaster)
{
	MyCaster = NewCaster;
	World = MyCaster->GetWorld();

	check(MyCaster);
	check(World);

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
			PressStartTime = World->GetTimeSeconds();
			CurrentState = EAbilityState::Pressed;

			if (MyCaster)
			{
				World->GetTimerManager().SetTimer(
					ThresholdTimerHandle,
					this,
					&UAbility::ThresholdMet,
					ClickDelay,
					false
				);
			}
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
void UAbility::RunEffect_Target(UGameplayEffect* Effect, ACharacter* Target)
{
	UEffectHandler* EffectHandler = Target->FindComponentByClass<UEffectHandler>();
	if (EffectHandler)
	{
		EffectHandler->AddEffect(Effect);
	}
}

TArray<ACharacter*> UAbility::RunEffect_AOE(UGameplayEffect* Effect, FVector Location, float Radius, ETargetSelection TargetSelection)
{
	//TODO: Change this so that all found actors gets the effect given on them
	//Use this:
	/*UEffectHandler* EffectHandler = Target->FindComponentByClass<UEffectHandler>();

	if (EffectHandler)
	{
		EffectHandler->YourFunction();
	}*/
	
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
		}
	}

	return Targets;
}

void UAbility::RunEffect_Projectile(FLatentActionInfo LatentInfo, UGameplayEffect* Effect, UStaticMesh* Mesh, FVector Target, float Speed, int32 PenetrationCount, FVector& OutLocation)
{
	if (Speed == 0.f)
		UE_LOG(LogTemp, Warning, TEXT("Projectile has 0 speed"));
	check(Mesh)
	
	if (!MyCaster || !World)
	{
		return;
	}

	FLatentActionManager& LAM = World->GetLatentActionManager();
	FEffect_ProjectileAction* ProjectileAction = new FEffect_ProjectileAction(LatentInfo, OutLocation);
	LAM.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, ProjectileAction);

	AProjectile* Projectile = World->SpawnActor<AProjectile>(
		AProjectile::StaticClass(),
		MyCaster->GetActorLocation(),
		MyCaster->GetActorRotation()
	);
	Projectile->SetMyAbility(this);
	Projectile->SetMyCaster(MyCaster);
	Projectile->Destination = Target;
	Projectile->Speed = Speed;
	Projectile->PenetrationCount = PenetrationCount;
	Projectile->MeshComponent->IgnoreActorWhenMoving(MyCaster, true);

	if (Mesh)
	{
		Projectile->MeshComponent->SetStaticMesh(Mesh);
	}

	TWeakObjectPtr<UAbility> WeakThis = this;
	Projectile->OnHit.BindLambda([WeakThis, ProjectileAction](FVector HitLocation)
	{
		if (WeakThis.IsValid())
		{
			ProjectileAction->Finish(HitLocation);
		}
	});
}



// Uncertain if I need
/*AActor* UAbility::RunEffect_SpawnObject(AActor* SpawnActor, FVector SpawnLocation)
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
}*/