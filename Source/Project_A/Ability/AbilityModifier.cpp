#include "AbilityModifier.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

AAbilityModifier::AAbilityModifier()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
}

void AAbilityModifier::BeginPlay()
{
	Super::BeginPlay();

	// Spawned with no placement of its own - put it at the caster so the Cast/Finish payloads
	// report a meaningful location.
	if (MyCaster)
	{
		SetActorLocation(MyCaster->GetActorLocation());
	}

	ReportCast();
	IAbilityLifecycle::Execute_OnActivate(this);

	// OnActivate may already have finished it.
	Finish();
}

void AAbilityModifier::Finish()
{
	if (bHasFinished)
	{
		return;
	}

	const FAbilityEventPayload Payload = NotifyFinish();

	IAbilityLifecycle::Execute_OnEnd(this, Payload.Location);

	Destroy();
}
