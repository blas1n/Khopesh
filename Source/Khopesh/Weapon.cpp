#include "Weapon.h"
#include "KhopeshAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"

AWeapon::AWeapon()
{
 	PrimaryActorTick.bCanEverTick = false;
	
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(RootComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void AWeapon::SetAttack(bool IsAttack)
{
	Capsule->SetActive(IsAttack);
}