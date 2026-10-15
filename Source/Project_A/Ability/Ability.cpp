#include "Ability.h"
#include "GrantedAbility.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "../Unit/UnitBase.h"
#include "../Unit/PlayerUnit.h"
#include "../Unit/EnemyUnit.h"
#include "../Component/EffectComponent.h"
#include "../Component/AbilityComponent.h"
#include "../Interfaces/AbilityLifecycle.h"

AAbility::AAbility()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	// No mass / no physics - query-only collision, overlap rather than block.
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	MeshComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	MeshComponent->SetGenerateOverlapEvents(true);
}

void AAbility::BeginPlay()
{
	Super::BeginPlay();

	checkf(AbilityName != FName("NO_NAME_ABILITY"),
		TEXT("%s has no AbilityName set - every ability must be given a unique name."), *GetClass()->GetName());

	AbilityTag = ComposeAbilityTag(AbilityName);

	if (FinishEventTags.IsEmpty())
	{
		FinishEventTags = ComposeEventTags(TEXT("Finish"));
	}
}

void AAbility::ReportCast()
{
	FAbilityEventPayload Payload;
	Payload.Ability = this;
	Payload.Location = GetActorLocation();

	ReportAbilityEvent(ComposeEventTags(TEXT("Cast")), Payload);
}

FGameplayTag AAbility::ComposeAbilityTag(FName InAbilityName, bool bErrorIfNotFound)
{
	return FGameplayTag::RequestGameplayTag(FName(*(FString(TEXT("Ability.")) + InAbilityName.ToString())), bErrorIfNotFound);
}

void AAbility::SetMyGrantedAbility(UGrantedAbility* GrantedAbility)
{
	MyGrantedAbility = GrantedAbility;
}

void AAbility::SetMyCaster(ACharacter* Caster)
{
	MyCaster = Caster;
}

void AAbility::ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload)
{
	if (Payload.Ability == nullptr)
	{
		Payload.Ability = this;
	}
	if (MyGrantedAbility)
	{
		MyGrantedAbility->ReportAbilityEvent(EventTags, Payload);
	}
}

FGameplayTagContainer AAbility::ComposeEventTags(const TCHAR* Suffix) const
{
	if (!AbilityTag.IsValid())
	{
		return FGameplayTagContainer();
	}

	FGameplayTagContainer ComposedTags;
	ComposedTags.AddTag(AbilityTag);

	const FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(FName(*(FString(TEXT("Event.")) + Suffix)));
	if (EventTag.IsValid())
	{
		ComposedTags.AddTag(EventTag);
	}

	return ComposedTags;
}

bool AAbility::MatchesSelection(const AUnitBase* Unit, ETargetSelection TargetSelection)
{
	switch (TargetSelection)
	{
	case ETargetSelection::PlayerUnit:
		return Unit->IsA<APlayerUnit>();
	case ETargetSelection::EnemyUnit:
		return Unit->IsA<AEnemyUnit>();
	case ETargetSelection::All:
	default:
		return true;
	}
}

AUnitBase* AAbility::Target_Single(ETargetSelection TargetSelection, FVector Location)
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(TargetAcceptanceRadius);
	FCollisionQueryParams Params;

	AUnitBase* Target = nullptr;
	float ClosestDistSq = 0.f;

	if (GetWorld() && GetWorld()->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (FOverlapResult& Overlap : Overlaps)
		{
			AUnitBase* Unit = Cast<AUnitBase>(Overlap.GetActor());
			if (!Unit || !MatchesSelection(Unit, TargetSelection))
			{
				continue;
			}

			const float DistSq = FVector::DistSquared(Unit->GetActorLocation(), Location);
			if (!Target || DistSq < ClosestDistSq)
			{
				Target = Unit;
				ClosestDistSq = DistSq;
			}
		}
	}

	return Target;
}

TArray<AUnitBase*> AAbility::Target_AOE(ETargetSelection TargetSelection, FVector Location, float Radius)
{
	TArray<AUnitBase*> FoundUnits;
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;

	if (GetWorld() && GetWorld()->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_Pawn, Sphere, Params))
	{
		for (FOverlapResult& Overlap : Overlaps)
		{
			AUnitBase* Unit = Cast<AUnitBase>(Overlap.GetActor());
			if (Unit && MatchesSelection(Unit, TargetSelection))
			{
				FoundUnits.Add(Unit);
			}
		}
	}

	return FoundUnits;
}

void AAbility::OnHit(AUnitBase* Target, FVector Location)
{
	// AAbility itself doesn't implement IAbilityLifecycle - its concrete subclasses (AProjectile,
	// APersistentAbility) do.
	// While the hook runs, ApplyEffect records what it applies so the hit's payload can carry it.
	bResolvingHit = true;
	HitAppliedEffect.Reset();
	if (Implements<UAbilityLifecycle>())
	{
		IAbilityLifecycle::Execute_OnTargetHit(this, Target, Location);
	}
	bResolvingHit = false;

	FAbilityEventPayload Payload;
	Payload.Ability = this;
	Payload.Target = Target;
	Payload.Location = Location;
	if (HitAppliedEffect.IsSet())
	{
		Payload.AppliedEffect = HitAppliedEffect.GetValue();
	}

	if (HitEventTags.IsEmpty())
	{
		HitEventTags = ComposeEventTags(TEXT("TargetHit"));
	}

	// Added to a copy - HitEventTags is reused for every hit.
	FGameplayTagContainer ReportedTags = HitEventTags;
	if (Payload.AppliedEffect.bCrit)
	{
		ReportedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.Crit")));
	}

	ReportAbilityEvent(ReportedTags, Payload);
	OnHitDelegate.Broadcast(Payload);
}

void AAbility::ApplyEffect(AUnitBase* Target, const FAttributeEffect& Effect)
{
	UEffectComponent* EffectComponent = Target->FindComponentByClass<UEffectComponent>();
	if (!EffectComponent)
	{
		return;
	}

	const FAttributeEffect Applied = EffectComponent->ApplyEffect(Effect);

	// Only the first effect of a hit is recorded - the payload has room for one.
	if (bResolvingHit && !HitAppliedEffect.IsSet())
	{
		HitAppliedEffect = Applied;
	}
}

FAttributeEffect AAbility::GetEffect(FGameplayTag FromAbility) const
{
	const AUnitBase* Caster = Cast<AUnitBase>(MyCaster);
	if (!Caster || !Caster->AbilityComponent)
	{
		return FAttributeEffect();
	}

	UGrantedAbility* GrantedAbility = Caster->AbilityComponent->FindGrantedAbility(FromAbility);
	return GrantedAbility ? GrantedAbility->GetEffect() : FAttributeEffect();
}

void AAbility::Finish()
{
	if (bHasFinished)
	{
		return;
	}

	NotifyFinish();

	Destroy();
}

FAbilityEventPayload AAbility::NotifyFinish()
{
	bHasFinished = true;

	FAbilityEventPayload Payload;
	Payload.Ability = this;
	Payload.Location = GetActorLocation();

	ReportAbilityEvent(FinishEventTags, Payload);
	OnFinish.Broadcast(Payload);

	return Payload;
}
