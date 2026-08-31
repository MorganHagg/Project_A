#include "Ability.h"
#include "../Unit/UnitBase.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Projectile.h"
#include "AbilityActor.h"
#include "../Misc/GameplayEffect.h"
#include "../Component/EffectHandler.h"
#include "DSP/SpectrumAnalyzer.h"


// TODO:
// EndAbility resets the ability, doesn't destroy it.
// Talents lives as UObjects
// Talents are read into an "Available Talents" array during game-start, and read from this when rolling talents
// (Can easily remove talents from the array as they get picked, or if some conflict with each other)
UAbility::UAbility()
{
	
}

float UAbility::GetCoolDown()
{
	return CoolDown;
}

float UAbility::GetCost()
{
	return Cost;
}

float UAbility::GetMagnitude()
{
	return MagnitudeMultiplier;
}

void UAbility::SetupAbility(AUnitBase* NewCaster)
{
	MyCaster = NewCaster;
	World = MyCaster->GetWorld();

	// Checks
	check(MyCaster);
	check(World);
}

void UAbility::ActivateAbility()
{
	IAbilityLifecycle::Execute_OnActivate(this);
}

void UAbility::TickAbility(float DeltaTime)
{
	if (Ticker.ShouldTick(DeltaTime))
		IAbilityLifecycle::Execute_OnTick(this);
}


void UAbility::EndAbility()
{
	if (bHasEnded) return;
	bHasEnded = true;
	
	IAbilityLifecycle::Execute_OnEnd(this);
}

void UAbility::KillAbility()
{
	MyCaster = nullptr;
}

// ============================================================================
// Execute library
// ============================================================================
void UAbility::Execute_Target(const FGameplayEffect& Effect, AUnitBase* Target)
{
	UEffectHandler* EffectHandler = Target->FindComponentByClass<UEffectHandler>();
	if (EffectHandler)
	{
		DelegateOnHit(Target);
		EffectHandler->AddEffect(Effect);
	}
}

TArray<ACharacter*> UAbility::Execute_AOE(const FGameplayEffect& Effect, FVector Location, float Radius, ETargetSelection TargetSelection)
{
	TArray<ACharacter*> Targets;
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	
	//TODO: Check if TargetCharacter implements UEffectHandler, and if it exists. Else return
	if (World && World->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (FOverlapResult& Overlap : Overlaps)
		{
			AUnitBase* TargetCharacter = Cast<AUnitBase>(Overlap.GetActor());
			if (!TargetCharacter)
			{
				continue;
			}
			//TODO: Update this so it checks whether the AOE should target friendly, hostile or all (From the perspective of the caster)
			if (TargetSelection == ETargetSelection::All)
			{
				if (UEffectHandler* EffectHandler = TargetCharacter->FindComponentByClass<UEffectHandler>())
				{
					EffectHandler->AddEffect(Effect);
				}
				DelegateOnHit(TargetCharacter);
				Targets.AddUnique(TargetCharacter);
			}
		}
	}

	return Targets;
}

void UAbility::Execute_Projectile(FLatentActionInfo LatentInfo, const FGameplayEffect& Effect, UStaticMesh* Mesh, FVector Target, float Speed, int32 PenetrationCount, FVector& OutLocation)
{
	if (Speed == 0.f)
		UE_LOG(LogTemp, Warning, TEXT("Projectile has 0 speed"));
	check(Mesh);

	AProjectile* Projectile = World->SpawnActor<AProjectile>(
	AProjectile::StaticClass(),
	MyCaster->GetActorLocation(),
	MyCaster->GetActorRotation()
);
	if (!Projectile)
	{
		return;
	}
	
	Projectile->SetMyAbility(this);
	Projectile->SetMyCaster(MyCaster);
	Projectile->MyEffect = Effect;
	Projectile->Destination = Target;
	Projectile->Speed = Speed;
	Projectile->PenetrationCount = PenetrationCount;
	Projectile->MeshComponent->SetStaticMesh(Mesh);
	Projectile->MeshComponent->IgnoreActorWhenMoving(MyCaster, true);
	
	FLatentActionManager& LAM = World->GetLatentActionManager();
	FEffect_ProjectileAction* ProjectileAction = new FEffect_ProjectileAction(LatentInfo, OutLocation);
	static int32 ProjectileActionUUIDCounter = LatentInfo.UUID;
	int32 UniqueUUID = ProjectileActionUUIDCounter++;

	LAM.AddNewAction(LatentInfo.CallbackTarget, UniqueUUID, ProjectileAction);

	Projectile->OnFinished.BindLambda([ProjectileAction](FVector HitLocation)
{
	ProjectileAction->Finish(HitLocation);
});
}

AAbilityActor* UAbility::Execute_Summon(const FGameplayEffect& Effect, TSubclassOf<AAbilityActor> NewActor, FTransform Transform)
{
	if (!NewActor || !World)
	{
		return nullptr;
	}

	AAbilityActor* AbilityActor = World->SpawnActor<AAbilityActor>(
		NewActor,
		Transform
	);

	if (AbilityActor)
	{
		AbilityActor->SetMyAbility(this);
		AbilityActor->SetMyCaster(MyCaster);
		AbilityActor->MyEffect = Effect;
	}
	return AbilityActor;
}


void UAbility::DelegateOnHit(AUnitBase* Target)
{
	OnAbilityHit(Target);
	OnHit.Broadcast(this, Target);
}