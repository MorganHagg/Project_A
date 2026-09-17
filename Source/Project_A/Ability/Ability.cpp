#include "Ability.h"
#include "AbilitySlot.h"
#include "Components/StaticMeshComponent.h"

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

	AbilityTag = FGameplayTag::RequestGameplayTag(FName(*(FString(TEXT("Ability.")) + AbilityName.ToString())));
}

void AAbility::SetMyAbility(UAbilitySlot* Ability)
{
	MyAbility = Ability;
}

void AAbility::SetMyCaster(ACharacter* Caster)
{
	MyCaster = Caster;
}

void AAbility::ReportAbilityEvent(FGameplayTagContainer EventTags, FAbilityEventPayload Payload)
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

void AAbility::HitTarget_Implementation(AUnitBase* Target, FVector Location)
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

void AAbility::ApplyEffect(AUnitBase* Target, const FGameplayEffect& Effect)
{
	if (MyAbility)
	{
		MyAbility->ApplyEffect(Target, Effect);
	}
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
	Payload.Ability = MyAbility;
	Payload.AbilityProduct = this;
	Payload.Location = GetActorLocation();

	ReportAbilityEvent(FinishEventTags, Payload);
	OnFinish.Broadcast(Payload);

	return Payload;
}
