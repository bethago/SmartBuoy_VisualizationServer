#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "TwinDeviceActor.h"
#include "TwinDeviceServer.generated.h"

UCLASS()
class VISUALIZATIONSERVER_API ATwinDeviceServer : public AActor
{
	GENERATED_BODY()

public:
	ATwinDeviceServer();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void StartTCPServer();
	void ListenForConnections();
	void HandleClient(FSocket* ClientSocket);
	bool IsLocationValid(FVector NewLocation, float MinDistance = 500.0f);

	FSocket* ListenerSocket;
	TArray<FSocket*> ClientSockets;

	UPROPERTY(EditAnywhere)
	int32 ServerPort = 10002;

	UPROPERTY(EditAnywhere)
	TMap<FString, ATwinDeviceActor*> TwinDeviceActors;
};
