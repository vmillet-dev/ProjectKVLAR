// Copyright Epic Games, Inc. All Rights Reserved.

#include "KUPGameplayTags.h"

// The string passed to each macro is the tag's full name as it appears in the editor's tag picker.
namespace KUPTags
{
	UE_DEFINE_GAMEPLAY_TAG(Element_Fire, "Element.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Element_Ice, "Element.Ice");
	UE_DEFINE_GAMEPLAY_TAG(Element_Poison, "Element.Poison");
	UE_DEFINE_GAMEPLAY_TAG(Element_Lightning, "Element.Lightning");

	UE_DEFINE_GAMEPLAY_TAG(Form_Projectile, "Form.Projectile");
	UE_DEFINE_GAMEPLAY_TAG(Form_Nova, "Form.Nova");

	UE_DEFINE_GAMEPLAY_TAG(Modifier_Overcharged, "Modifier.Overcharged");
	UE_DEFINE_GAMEPLAY_TAG(Modifier_Unstable, "Modifier.Unstable");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Cast_Left, "Ability.Cast.Left");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Cast_Right, "Ability.Cast.Right");

	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");

	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");

	UE_DEFINE_GAMEPLAY_TAG(Status_Burn, "Status.Burn");
	UE_DEFINE_GAMEPLAY_TAG(Status_Freeze, "Status.Freeze");
	UE_DEFINE_GAMEPLAY_TAG(Status_Poisoned, "Status.Poisoned");
}
