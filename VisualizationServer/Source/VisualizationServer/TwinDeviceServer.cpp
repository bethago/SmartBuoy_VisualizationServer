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
        ReceivedString = ReceivedString.TrimStartAndEnd();
        
        int32 ClosingBraceIndex;
        if (ReceivedString.FindLastChar('}', ClosingBraceIndex))
        {
            ReceivedString = ReceivedString.Left(ClosingBraceIndex + 1);
        }

        UE_LOG(LogTemp, Log, TEXT("Processed JSON: %s"), *ReceivedString);
        
        try
        {
            json jsonData = json::parse(TCHAR_TO_UTF8(*ReceivedString));

            
            if (!jsonData.contains("dn") || !jsonData.contains("ds") || !jsonData.contains("gps"))
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid JSON: Missing required fields"));
                return;
            }

            FString DeviceName = FString(jsonData["dn"].get<std::string>().c_str());
            int DangerState = jsonData["ds"].get<int>();

            if (!jsonData["gps"].contains("lat") || !jsonData["gps"].contains("lon") || !jsonData["gps"].contains("alt"))
            {
                UE_LOG(LogTemp, Error, TEXT("Invalid JSON: Missing GPS fields"));
                return;
            }

            FVector GPSPosition(
                jsonData["gps"]["lon"].get<float>(), 
                jsonData["gps"]["lat"].get<float>(), 
                jsonData["gps"]["alt"].get<float>()
            );

            int MaxAttempts = 10;
            int Attempts = 0;

            while (!IsLocationValid(GPSPosition) && Attempts < MaxAttempts)
            {
                GPSPosition += FVector(FMath::RandRange(50, 100), FMath::RandRange(50, 100), 0); 
                Attempts++;
            }

            if (!TwinDeviceActors.Contains(DeviceName))
            {
                FActorSpawnParameters SpawnParams;
                // SpawnParams.Owner = this;
                ATwinDeviceActor* NewTwinDevice = GetWorld()->SpawnActor<ATwinDeviceActor>(ATwinDeviceActor::StaticClass(), GPSPosition, FRotator::ZeroRotator, SpawnParams);
                if (NewTwinDevice)
                {
                    TwinDeviceActors.Add(DeviceName, NewTwinDevice);
                }
            }

            if (TwinDeviceActors.Contains(DeviceName))
            {
                TwinDeviceActors[DeviceName]->UpdateTwinDeviceState(GPSPosition, DangerState, DeviceName);
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

bool ATwinDeviceServer::IsLocationValid(FVector NewLocation, float MinDistance)
{
    for (const auto& Pair : TwinDeviceActors)
    {
        if (FVector::Dist(Pair.Value->GetActorLocation(), NewLocation) < MinDistance)
        {
            return false;
        }
    }
    return true;
}