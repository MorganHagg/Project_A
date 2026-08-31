#include "UnitDataBase.h"

UUnitDataBase::UUnitDataBase()
{
	DefaultAttributes = {
		{ EAttributeType::Health, 100.f },
		{ EAttributeType::Strength, 10.f },
		{ EAttributeType::Intellect, 10.f },
		{ EAttributeType::Armour, 5.f },
		{ EAttributeType::MagicResist, 5.f },
		{ EAttributeType::Speed, 100.f },
		{ EAttributeType::Crit, 5.f }
	};
}