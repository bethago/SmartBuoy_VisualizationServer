#include "TwinDeviceActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ATwinDeviceActor::ATwinDeviceActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/StarterContent/Props/MaterialSphere"));
	if (MeshAsset.Succeeded())
	{
		MeshComponent->SetStaticMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Game/BaseMaterial"));
	if (MaterialAsset.Succeeded())
	{
		MeshComponent->SetMaterial(0, MaterialAsset.Object);
	}
}

void ATwinDeviceActor::BeginPlay()
{
	Super::BeginPlay();

	DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
}

void ATwinDeviceActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATwinDeviceActor::UpdateTwinDeviceState(FVector NewGPS, int NewDangerState)
{
	GPSPosition = NewGPS;
	DangerState = NewDangerState;
	
	SetActorLocation(GPSPosition, false, nullptr, ETeleportType::TeleportPhysics);

	if (DynamicMaterial)
	{
		FLinearColor Color;
		switch (DangerState)
		{
		case 0:  // Normal
			Color = FLinearColor::Green;
			break;
		case 1:  // Warning
			Color = FLinearColor::Yellow;
			break;
		case 2:  // Danger
			Color = FLinearColor::Red;
			break;
		default:
			Color = FLinearColor::White;
			break;
		}
		DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), Color);
	}
	// UE_LOG(LogTemp, Log, TEXT("Updated TwinDeviceActor: New GPS: (%f, %f, %f), Danger Level: %d"), GPSPosition.X, GPSPosition.Y, GPSPosition.Z, DangerState);
}
