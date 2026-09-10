#include "Ability.h"
#include "../Unit/UnitBase.h"
#include "../Unit/PlayerUnit.h"
#include "Gameframework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "Projectile.h"
#include "AbilityActor.h"
#include "../Misc/GameplayEffect.h"
#include "../Component/EffectHandler.h"

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
	checkf(AbilityName != FName("NO_NAME_ABILITY"),
		TEXT("%s has no AbilityName set - every ability must be given a unique name."), *GetClass()->GetName());

	AbilityTag = FGameplayTag::RequestGameplayTag(FName(*(FString(TEXT("Ability.")) + AbilityName.ToString())));
}

void UAbility::ActivateAbility()
{
	bHasEnded = false;

	FAbilityEventPayload CastPayload;
	CastPayload.Location = MyCaster ? MyCaster->GetActorLocation() : FVector::ZeroVector;
	ReportAbilityEvent(ComposeEventTag(TEXT("Cast")), CastPayload);

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

	FAbilityEventPayload FinishPayload;
	FinishPayload.Location = MyCaster ? MyCaster->GetActorLocation() : FVector::ZeroVector;
	ReportAbilityEvent(ComposeEventTag(TEXT("Finish")), FinishPayload);
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
		EffectHandler->AddEffect(Effect);

		FAbilityEventPayload Payload;
		Payload.Target = Target;
		Payload.Location = Target->GetActorLocation();
		Payload.AppliedEffect = Effect;
		Payload.Magnitude = Effect.Magnitude;
		ReportAbilityEvent(ComposeEventTag(TEXT("TargetHit")), Payload);
	}
}

TArray<ACharacter*> UAbility::Execute_AOE(
	const FGameplayEffect& Effect, FVector Location, float Radius,
	ETargetSelection TargetSelection)
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

				FAbilityEventPayload Payload;
				Payload.Target = TargetCharacter;
				Payload.Location = TargetCharacter->GetActorLocation();
				Payload.AppliedEffect = Effect;
				Payload.Magnitude = Effect.Magnitude;
				ReportAbilityEvent(ComposeEventTag(TEXT("TargetHit")), Payload);

				Targets.AddUnique(TargetCharacter);
			}
		}
	}

	return Targets;
}

void UAbility::Execute_Projectile(
	FLatentActionInfo LatentInfo, const FGameplayEffect& Effect, UStaticMesh* Mesh,
	FVector Target, float Speed, int32 PenetrationCount, FVector& OutLocation)
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
	Projectile->HitEventTag = ComposeEventTag(TEXT("TargetHit"));
	Projectile->FinishEventTag = ComposeEventTag(TEXT("Finish"));
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
		AbilityActor->SetLifeSpan(Effect.Duration);

		if (!AbilityActor->FinishEventTag.IsValid())
		{
			AbilityActor->FinishEventTag = ComposeEventTag(TEXT("Finish"));
		}
	}
	return AbilityActor;
}


void UAbility::ReportAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload)
{
	Payload.Ability = this;
	if (APlayerUnit* PlayerUnit = Cast<APlayerUnit>(MyCaster))
	{
		PlayerUnit->BroadcastAbilityEvent(EventTag, Payload);
	}
}

FGameplayTag UAbility::ComposeEventTag(const TCHAR* Suffix) const
{
	if (!AbilityTag.IsValid())
	{
		return FGameplayTag();
	}
	return FGameplayTag::RequestGameplayTag(FName(*(AbilityTag.ToString() + TEXT(".") + Suffix)));
}