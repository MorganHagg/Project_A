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

void AAbilityProduct::ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload)
{
	Payload.Ability = MyAbility;
	if (Payload.AbilityProduct == nullptr)
	{
		Payload.AbilityProduct = this;
	}
	if (MyAbility)
	{
		MyAbility->ReportAbilityEvent(EventTags, Payload);
	}
}

void AAbilityProduct::HitTarget_Implementation(AUnitBase* Target, FVector Location)
{
	FAbilityEventPayload Payload;
	Payload.Ability = MyAbility;
	Payload.AbilityProduct = this;
	Payload.Target = Target;
	Payload.Location = Location;

	if (HitEventTags.IsEmpty() && MyAbility)
	{
		HitEventTags = MyAbility->ComposeEventTags(TEXT("TargetHit"));
	}

	ReportAbilityEvent(HitEventTags, Payload);
	OnHit.Broadcast(Payload);
}

void AAbilityProduct::ApplyEffect(AUnitBase* Target, const FGameplayEffect& Effect)
{
	if (MyAbility)
	{
		MyAbility->ApplyEffect(Target, Effect);
	}
}

void AAbilityProduct::Finish()
{
	if (bHasFinished)
	{
		return;
	}

	NotifyFinish();

	Destroy();
}

FAbilityEventPayload AAbilityProduct::NotifyFinish()
{
	bHasFinished = true;

	FAbilityEventPayload Payload;
	Payload.Ability = MyAbility;
	Payload.AbilityProduct = this;
	Payload.Location = GetActorLocation();

	ReportAbilityEvent(FinishEventTags, Payload);
	OnFinish.Broadcast(Payload);

	return Payload;
}
