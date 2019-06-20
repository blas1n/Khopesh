#include "Weapon.h"
#include "KhopeshAnimInstance.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

AWeapon::AWeapon()
{
 	PrimaryActorTick.bCanEverTick = false;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetupAttachment(RootComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void AWeapon::SetAttack(bool IsAttack)
{
	Box->SetActive(IsAttack);
}