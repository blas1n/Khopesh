#include "Weapon.h"
#include "KhopeshAnimInstance.h"
#include "Components/CapsuleComponent.h"

AWeapon::AWeapon()
{
 	PrimaryActorTick.bCanEverTick = false;
	//Capsule->SetAutoActivate(false);
}

void AWeapon::SetAttack(bool IsAttack)
{
	Capsule->SetActive(IsAttack);
}