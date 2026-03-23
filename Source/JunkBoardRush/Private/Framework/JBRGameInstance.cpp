#include "Framework/JBRGameInstance.h"

#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Engine/Engine.h" // added for GEngine and AddOnScreenDebugMessage
#include "Kismet/GameplayStatics.h"

UJBRGameInstance::UJBRGameInstance()
{
}

void UJBRGameInstance::Init()
{
    Super::Init();

    // Get the Online Subsystem
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            // Assign callbacks
            SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UJBRGameInstance::OnCreateSessionComplete);
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UJBRGameInstance::OnFindSessionsComplete);
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UJBRGameInstance::OnJoinSessionComplete);
            
            SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UJBRGameInstance::OnDestroySessionComplete);
            
            if (GEngine)
            {
                GEngine->OnNetworkFailure().AddUObject(this, &UJBRGameInstance::OnNetworkFailure);
            }
        }
    }
}

void UJBRGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Network Error: %s"), *ErrorString));
    }
    
    LeaveGame();
}

void UJBRGameInstance::LeaveGame()
{
    if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        bIsDestroyingSession = true; 
        bIsNetworkBusy = true;
        SessionInterface->DestroySession(NAME_GameSession);
    }
    
    UGameplayStatics::OpenLevel(this, FName("MainMenu")); 
}

void UJBRGameInstance::HostSession(FString targetSessionCode, bool bSearchSessionOnSameNetwork)
{
    if (!SessionInterface.IsValid()) return;

    /*// Check if a session already exists (to avoid hosting multiple sessions on the same computer which Steam doesn't like)
    auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession); // This won't actually destroy other remote players' current sessions, it checks for LOCAL session on the machine
    if (ExistingSession != nullptr)
    {
        SessionInterface->DestroySession(NAME_GameSession);
    }*/

    FOnlineSessionSettings SessionSettings;

    // When searching/connecting only on the same network, use a LAN session
    SessionSettings.bIsLANMatch = bSearchSessionOnSameNetwork;

    // If we are in LAN mode, presence / lobbies / Steam-specific features are typically not required and can cause issues depending on subsystem.
    SessionSettings.bUsesPresence = !bSearchSessionOnSameNetwork;
    SessionSettings.bUseLobbiesIfAvailable = !bSearchSessionOnSameNetwork;

    SessionSettings.NumPublicConnections = 6;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bAllowJoinInProgress = true;

    FString fullSessionKey = FString("SessionJBR") + targetSessionCode;
    
    // Set a custom key to identify JBR, to prevents joining other devs games using AppID 480 (which is the free one Steamworks provides)
    SessionSettings.Set(SEARCH_KEYWORDS, fullSessionKey, EOnlineDataAdvertisementType::ViaOnlineService);

    SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void UJBRGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Session created successfully!"));
        }
        
        FString MapToTravel = "/Game/Maps/Arena_Map?listen";
        if (bCurrentSearchSessionOnSameNetwork)
        {
            MapToTravel.Append("?bIsLanMatch=1");
        }
        
        // Travel to the map as a listen server
        GetWorld()->ServerTravel(MapToTravel, true);
        bIsNetworkBusy = false;
        
        // print current connection string
        FString ConnectionString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectionString))
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Hosting session at %s"), *ConnectionString));

        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to create session!"));
        }
    }
}

void UJBRGameInstance::FindAndJoinSession(FString targetSessionCode, bool bSearchSessionOnSameNetwork)
{
    if (!SessionInterface.IsValid()) return;

    currentTargetSessionCode = targetSessionCode;
    bCurrentSearchSessionOnSameNetwork = bSearchSessionOnSameNetwork;
    
    bIsNetworkBusy = true;
    
    if (bIsDestroyingSession)
    {
        bWantsToSearchAfterDestroy = true; // Queue
        return;
    }

    if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        // Wait for destruction to finish before searching
        bWantsToSearchAfterDestroy = true;
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Destroying old session before searching..."));
        bIsDestroyingSession = true;
        SessionInterface->DestroySession(NAME_GameSession);
        return;
    }

    InternalFindSession();
}

void UJBRGameInstance::InternalFindSession()
{
    bIsNetworkBusy = true;
    
    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = 100;
    
    // LAN vs Online search depending on the flag
    SessionSearch->bIsLanQuery = bCurrentSearchSessionOnSameNetwork;

    if (!bCurrentSearchSessionOnSameNetwork)
    {
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
    }
    
    FString fullSessionKey = FString("SessionJBR") + currentTargetSessionCode;
    
    // Filter for specific game keyword
    SessionSearch->QuerySettings.Set(SEARCH_KEYWORDS, fullSessionKey, EOnlineComparisonOp::Equals);

    if (GEngine)
    {
        const FString ModeText = bCurrentSearchSessionOnSameNetwork ? TEXT("Finding LAN sessions on same network...") : TEXT("Finding sessions via online service...");
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, ModeText);
    }
    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UJBRGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Old session destroyed successfully!"));
    
    bIsDestroyingSession = false;
    bIsNetworkBusy = false;
    
    if (bWantsToSearchAfterDestroy)
    {
        bWantsToSearchAfterDestroy = false;
        InternalFindSession();
    }
}

void UJBRGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (bWasSuccessful && SessionSearch.IsValid())
    {
        if (GEngine)
        {
            const FString ModeText = bCurrentSearchSessionOnSameNetwork ? TEXT("LAN sessions found check") : TEXT("Online sessions found check");
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, ModeText);
        }

        // In all found sessions, search for one with open slot
        for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
        {
            if (SearchResult.IsValid() && SearchResult.Session.NumOpenPublicConnections > 0)
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("Found open session: %s. Ping: %d"), *SearchResult.GetSessionIdStr(), SearchResult.PingInMs));
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, TEXT("Joining"));
                }

                // Join the first open session found
                SessionInterface->JoinSession(0, NAME_GameSession, SearchResult);
                return; 
            }
            else
            {
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("Session found but full: %s"), *SearchResult.GetSessionIdStr()));
                }
            }
        }
    }

    // All sessions full or no sessions found, host a new one
    if (GEngine)
    {
        const FString ModeText = bCurrentSearchSessionOnSameNetwork ? TEXT("No open LAN sessions. Hosting new.") : TEXT("No open online sessions. Hosting new.");
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, ModeText);
    }

    // Host a new overflow session
    HostSession(currentTargetSessionCode, bCurrentSearchSessionOnSameNetwork);
}

void UJBRGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("Joined session successfully!"));
        }
        // The engine handles travel automatically. The connection string is resolved by the subsystem.
        APlayerController* PlayerController = GetFirstLocalPlayerController();
        if (PlayerController)
        {
            FString ConnectionString;
            if (SessionInterface->GetResolvedConnectString(SessionName, ConnectionString)) // Gets the network address of the session that the client must connect to, if this returns true that means the session is valid
            {
                PlayerController->ClientTravel(ConnectionString, ETravelType::TRAVEL_Absolute);
                bIsNetworkBusy = false;
                
                GEngine->AddOnScreenDebugMessage(-1, 500.0f, FColor::Cyan, FString::Printf(TEXT("Traveling to session at %s"), *ConnectionString));
            }
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Failed to join session."));
        }
    }
}