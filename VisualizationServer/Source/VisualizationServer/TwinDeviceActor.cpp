#include "TwinDeviceActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
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

	BuoyNameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("BuoyNameText"));
	BuoyNameText->SetupAttachment(MeshComponent);
	BuoyNameText->SetHorizontalAlignment(EHTA_Center);
	BuoyNameText->SetVerticalAlignment(EVRTA_TextCenter);
	BuoyNameText->SetWorldSize(50.0f);  // 텍스트 크기 설정
	BuoyNameText->SetTextRenderColor(FColor::White);
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

void ATwinDeviceActor::UpdateTwinDeviceState(FVector NewGPS, int NewDangerState, FString NewDeviceName)
{
	DeviceName = NewDeviceName;
	GPSPosition = NewGPS;
	DangerState = NewDangerState;
	
	SetActorLocation(GPSPosition, false, nullptr, ETeleportType::TeleportPhysics);

	BuoyNameText->SetText(FText::FromString(DeviceName));
	FVector TextOffset = FVector(0, 0, 100);
	BuoyNameText->SetWorldLocation(GPSPosition + TextOffset);

	FLinearColor Color;
	switch (DangerState)
	{
	case 0:  // SAFE
		Color = FLinearColor::Green;
		break;
	case 1:  // CAUTION
		Color = FLinearColor::Blue;
		break;
	case 2:  // ALERT
		Color = FLinearColor::Yellow;
		break;
	case 3:  // DANGER
		Color = FLinearColor::Red;
		break;
	default:
		Color = FLinearColor::White;
		break;
	}

	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), Color);
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Updated TwinDeviceActor: GPS: (%f, %f, %f), Danger Level: %d [%s]"), *DeviceName, GPSPosition.X, GPSPosition.Y, GPSPosition.Z, DangerState, *Color.ToString());
}
