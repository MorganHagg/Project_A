#include "AbilityProduct.h"
#include "Ability.h"
#include "Components/StaticMeshComponent.h"

AAbilityProduct::AAbilityProduct()
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

void AAbilityProduct::SetMyAbility(UAbility* Ability)
{
	MyAbility = Ability;
}

void AAbilityProduct::SetMyCaster(ACharacter* Caster)
{
	MyCaster = Caster;
}

void AAbilityProduct::ReportAbilityEvent(FGameplayTag EventTag, FAbilityEventPayload Payload)
{
	Payload.Ability = MyAbility;
	Payload.AbilityProduct = this;
	if (MyAbility)
	{
		MyAbility->ReportAbilityEvent(EventTag, Payload);
	}
	OnHit.Broadcast(Payload);
}

FVector AAbilityProduct::Finish()
{
	if (bHasFinished)
	{
		return GetActorLocation();
	}
	bHasFinished = true;

	FAbilityEventPayload Payload;
	Payload.Ability = MyAbility;
	Payload.AbilityProduct = this;
	Payload.Location = GetActorLocation();
	Payload.AppliedEffect = MyEffect;

	ReportAbilityEvent(FinishEventTag, Payload);
	OnFinish.Broadcast(Payload);

	Destroy();
	return Payload.Location;
}
