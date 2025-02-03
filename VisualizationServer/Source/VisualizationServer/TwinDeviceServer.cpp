#include "TwinDeviceServer.h"
#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "JsonUtilities.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ATwinDeviceServer::ATwinDeviceServer()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATwinDeviceServer::BeginPlay()
{
    Super::BeginPlay();
    StartTCPServer();
}

void ATwinDeviceServer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ListenerSocket)
    {
        ListenerSocket->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket);
    }

    for (FSocket* Socket : ClientSockets)
    {
        if (Socket)
        {
            Socket->Close();
            ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void ATwinDeviceServer::StartTCPServer()
{
    FIPv4Endpoint Endpoint(FIPv4Address::Any, ServerPort);
    ListenerSocket = FTcpSocketBuilder(TEXT("TwinDeviceServer"))
        .AsReusable()
        .BoundToEndpoint(Endpoint)
        .Listening(8);

    if (ListenerSocket)
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            ListenForConnections();
        });
    }
}

void ATwinDeviceServer::ListenForConnections()
{
    if (ListenerSocket)
    {
        bool Pending;
        if (ListenerSocket->HasPendingConnection(Pending) && Pending)
        {
            FSocket* ClientSocket = ListenerSocket->Accept(TEXT("TwinDeviceClient"));
            if (ClientSocket)
            {
                ClientSockets.Add(ClientSocket);
                HandleClient(ClientSocket);
            }
        }
    }

    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
    {
        ListenForConnections();
    });
}

void ATwinDeviceServer::HandleClient(FSocket* ClientSocket)
{
    uint32 DataSize;
    while (ClientSocket->HasPendingData(DataSize))
    {
        TArray<uint8> ReceivedData;
        ReceivedData.SetNumUninitialized(DataSize);
        int32 BytesRead = 0;
        ClientSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);

        FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));
        UE_LOG(LogTemp, Log, TEXT("Received Data: %s"), *ReceivedString);

        try
        {
            json jsonData = json::parse(TCHAR_TO_UTF8(*ReceivedString));

            if (!jsonData.contains("deviceName") || !jsonData.contains("dangerState") || !jsonData.contains("gps"))
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid JSON: Missing required fields"));
                return;
            }

            FString DeviceName = FString(jsonData["deviceName"].get<std::string>().c_str());
            int DangerState = jsonData["dangerState"].get<int>();

            if (!jsonData["gps"].contains("latitude") || !jsonData["gps"].contains("longitude") || !jsonData["gps"].contains("altitude"))
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid JSON: Missing GPS fields"));
                return;
            }

            float Latitude = jsonData["gps"]["latitude"].get<float>();
            float Longitude = jsonData["gps"]["longitude"].get<float>();
            float Altitude = jsonData["gps"]["altitude"].get<float>();

            FVector GPSPosition = FVector(Longitude, Latitude, Altitude);
            UE_LOG(LogTemp, Log, TEXT("GPS Position: (%f, %f, %f)"), GPSPosition.X, GPSPosition.Y, GPSPosition.Z);

            if (TwinDevice)
            {
                TwinDevice->UpdateTwinDeviceState(GPSPosition, DangerState);
            }
        }
        catch (const nlohmann::json::exception& e)
        {
            UE_LOG(LogTemp, Error, TEXT("JSON Parsing Error: %s"), *FString(e.what()));
        }
        catch (...)
        {
            UE_LOG(LogTemp, Error, TEXT("An unknown error occurred during JSON parsing"));
        }
    }
}
