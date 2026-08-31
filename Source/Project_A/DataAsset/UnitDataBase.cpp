#include "UnitDataBase.h"

UUnitDataBase::UUnitDataBase()
{
	Attributes.SetNum(static_cast<int32>(EAttributeType::Count));
	
	/*Attributes = {
        10.f,  // Strength
        10.f,  // Intellect
        5.f,   // Armour
        5.f,   // MagicResist
        100.f, // Speed
        5.f    // Crit
    };*/
}