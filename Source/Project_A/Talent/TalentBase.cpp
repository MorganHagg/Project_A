#include "TalentBase.h"

#include "Project_A/Ability/Ability.h"
#include "Project_A/Ability/Projectile.h"

void UTalentBase::BindToAbility(UAbility* Ability)
{
	MyAbility = Ability;
	if (!Ability)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to bind Talent to a null Ability"));
		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("I am bound to %s"),
		*Ability->GetAbilityName().ToString()
	);

	Ability->OnHit.AddDynamic(this, &UTalentBase::HandleAbilityHit);
	Ability->OnOverlap.AddDynamic(this, &UTalentBase::HandleAbilityOverlap);

	OnSetup();
}

void UTalentBase::BindToProjectile(AProjectile* Projectile)
{
	MyProjectile = Projectile;
	MyAbility = MyProjectile->GetAbility();
	if (!Projectile)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to bind Talent to a null Projectile"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("I am bound to a Projectile"));

	Projectile->OnProjectileHit.AddDynamic(this, &UTalentBase::HandleProjectileHit);
	Projectile->OnProjectileFinish.AddDynamic(this, &UTalentBase::HandleProjectileFinish);

	OnSetup();
}