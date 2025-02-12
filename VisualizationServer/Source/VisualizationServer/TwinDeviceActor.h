#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TwinDeviceActor.generated.h"

UCLASS()
class VISUALIZATIONSERVER_API ATwinDeviceActor : public AActor
{
	GENERATED_BODY()

public:
	ATwinDeviceActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	void UpdateTwinDeviceState(FVector NewGPS, int NewDangerState, FString NewDeviceName);

private:
	UPROPERTY(VisibleAnywhere)
	FVector GPSPosition;

	UPROPERTY(VisibleAnywhere)
	int DangerState;

	UPROPERTY(VisibleAnywhere)
	FString DeviceName;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* DynamicMaterial;
	
	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* BuoyNameText;
};
