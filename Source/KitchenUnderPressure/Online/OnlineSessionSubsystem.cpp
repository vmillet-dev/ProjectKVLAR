// Fill out your copyright notice in the Description page of Project Settings.


#include "OnlineSessionSubsystem.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Settings/GameMapSettings.h"
#include "Settings/OnlineSettings.h"
#include "KitchenUnderPressure.h"

static const FName SESSION_NAME = NAME_GameSession;
static const FName KEY_GAMEKEY = FName(TEXT("GAMEKEY"));

void UOnlineSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld()))
    {
        SessionInterface = OSS->GetSessionInterface();
        UE_LOG(LogKitchenUnderPressure, Log, TEXT("[Online] Subsystem: %s"), *OSS->GetSubsystemName().ToString());
    }
    else
    {
        UE_LOG(LogKitchenUnderPressure, Error, TEXT("[Online] No OnlineSubsystem found"));
    }
}

void UOnlineSessionSubsystem::Deinitialize()
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionHandle);
    }
    Super::Deinitialize();
}

// ---------------------------------------------------------------- LOGIN
void UOnlineSessionSubsystem::Login(const FString& LoginToken)
{
    IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld());
    if (!OSS) { OnLoginDone.Broadcast(false); return; }

    IOnlineIdentityPtr Identity = OSS->GetIdentityInterface();
    if (!Identity.IsValid()) { OnLoginDone.Broadcast(false); return; }

    if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
    {
        bIsLoggedIn = true;
        OnLoginDone.Broadcast(true);
        return;
    }

    const UOnlineSettings* OnlineCfg = GetDefault<UOnlineSettings>();

    FOnlineAccountCredentials Creds;
    Creds.Type = OnlineCfg->LoginType;     // "Developer" by default
    Creds.Id = OnlineCfg->DevAuthAddress;  // Dev Auth Tool address for the Developer login type
    Creds.Token = LoginToken;

    Identity->OnLoginCompleteDelegates[0].AddUObject(
        this, &UOnlineSessionSubsystem::HandleLoginComplete);

    // Never log the token itself.
    UE_LOG(LogKitchenUnderPressure, Log, TEXT("[Online] Login attempt -> Type=%s Id=%s"), *Creds.Type, *Creds.Id);
    Identity->Login(0, Creds);
}

void UOnlineSessionSubsystem::HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful,
    const FUniqueNetId& UserId, const FString& Error)
{
    if (IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld()))
    {
        OSS->GetIdentityInterface()->ClearOnLoginCompleteDelegates(0, this);
    }
    bIsLoggedIn = bWasSuccessful;
    UE_LOG(LogKitchenUnderPressure, Log, TEXT("[Online] Login %s %s"),
        bWasSuccessful ? TEXT("OK") : TEXT("FAIL"), *Error);
    OnLoginDone.Broadcast(bWasSuccessful);
}

// ---------------------------------------------------------------- HOST
void UOnlineSessionSubsystem::HostGame(const FString& MapName, int32 MaxPlayers)
{
    if (!SessionInterface.IsValid()) { OnHostDone.Broadcast(false); return; }

    PendingMapName = MapName;
    PendingMaxPlayers = MaxPlayers;

    // Destroy any leftover session first, then create once it is actually gone.
    if (DestroyExistingSession(EPendingSessionOp::Host))
    {
        return;
    }
    DoCreateSession();
}

void UOnlineSessionSubsystem::DoCreateSession()
{
    if (!SessionInterface.IsValid()) { OnHostDone.Broadcast(false); return; }

    FOnlineSessionSettings Settings;
    Settings.NumPublicConnections = PendingMaxPlayers;
    Settings.bShouldAdvertise = true;
    Settings.bAllowJoinInProgress = true;
    Settings.bIsLANMatch = false;
    Settings.bUsesPresence = true;
    Settings.bUseLobbiesIfAvailable = true; // essential for EOS
    Settings.bAllowJoinViaPresence = true;

    Settings.Set(KEY_GAMEKEY, GetDefault<UOnlineSettings>()->GameKey,
        EOnlineDataAdvertisementType::ViaOnlineService);

    CreateSessionHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(
            this, &UOnlineSessionSubsystem::HandleCreateSessionComplete));

    SessionInterface->CreateSession(0, SESSION_NAME, Settings);
}

void UOnlineSessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
    UE_LOG(LogKitchenUnderPressure, Log, TEXT("[Online] CreateSession %s"),
        bWasSuccessful ? TEXT("OK") : TEXT("FAIL"));

    if (bWasSuccessful)
    {
        // The host opens the map as a listen server.
        const FString TravelURL = FString::Printf(TEXT("%s?listen"), *PendingMapName);
        GetWorld()->ServerTravel(TravelURL);
    }
    OnHostDone.Broadcast(bWasSuccessful);
}

// ---------------------------------------------------------------- FIND
void UOnlineSessionSubsystem::FindGames()
{
    if (!SessionInterface.IsValid()) { OnFindDone.Broadcast(0); return; }

    FoundSessions.Empty();

    SearchSettings = MakeShareable(new FOnlineSessionSearch());
    SearchSettings->MaxSearchResults = 20;
    SearchSettings->bIsLanQuery = false;
    SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    SearchSettings->QuerySettings.Set(KEY_GAMEKEY, GetDefault<UOnlineSettings>()->GameKey, EOnlineComparisonOp::Equals);

    FindSessionsHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(
            this, &UOnlineSessionSubsystem::HandleFindSessionsComplete));

    SessionInterface->FindSessions(0, SearchSettings.ToSharedRef());
}

void UOnlineSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsHandle);

    if (bWasSuccessful && SearchSettings.IsValid())
    {
        for (const FOnlineSessionSearchResult& Result : SearchSettings->SearchResults)
        {
            FGameSessionInfo Info;
            Info.OwnerName = Result.Session.OwningUserName;
            Info.MaxSlots = Result.Session.SessionSettings.NumPublicConnections;
            Info.OpenSlots = Result.Session.NumOpenPublicConnections;
            Info.PingMs = Result.PingInMs;
            FoundSessions.Add(Info);
        }
    }

    UE_LOG(LogKitchenUnderPressure, Log, TEXT("[Online] Find: %d sessions"), FoundSessions.Num());
    OnFindDone.Broadcast(FoundSessions.Num());
}

// ---------------------------------------------------------------- JOIN
void UOnlineSessionSubsystem::JoinGame(int32 SearchResultIndex)
{
    if (!SessionInterface.IsValid() || !SearchSettings.IsValid()) { OnJoinDone.Broadcast(false); return; }
    if (!SearchSettings->SearchResults.IsValidIndex(SearchResultIndex)) { OnJoinDone.Broadcast(false); return; }

    PendingJoinIndex = SearchResultIndex;

    // Symmetric with hosting: drop any stale session before joining, otherwise EOS rejects
    // the join with "already in session".
    if (DestroyExistingSession(EPendingSessionOp::Join))
    {
        return;
    }
    DoJoinSession();
}

void UOnlineSessionSubsystem::DoJoinSession()
{
    if (!SessionInterface.IsValid() || !SearchSettings.IsValid()
        || !SearchSettings->SearchResults.IsValidIndex(PendingJoinIndex))
    {
        OnJoinDone.Broadcast(false);
        return;
    }

    JoinSessionHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(
            this, &UOnlineSessionSubsystem::HandleJoinSessionComplete));

    SessionInterface->JoinSession(0, SESSION_NAME,
        SearchSettings->SearchResults[PendingJoinIndex]);
}

void UOnlineSessionSubsystem::HandleJoinSessionComplete(FName SessionName,
    EOnJoinSessionCompleteResult::Type Result)
{
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);

    // AlreadyInSession is recoverable: we are in the session, so just resolve and travel.
    const bool bJoined = (Result == EOnJoinSessionCompleteResult::Success
                       || Result == EOnJoinSessionCompleteResult::AlreadyInSession);

    bool bTravelled = false;
    if (bJoined)
    {
        // GetResolvedConnectString resolves the EOS P2P address cleanly.
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SESSION_NAME, ConnectString))
        {
            if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
            {
                PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
                bTravelled = true;
            }
        }
        else
        {
            UE_LOG(LogKitchenUnderPressure, Error, TEXT("[Online] ConnectString not found"));
        }
    }

    UE_LOG(LogKitchenUnderPressure, Log, TEXT("[Online] JoinSession result=%d travelled=%d"),
        static_cast<int32>(Result), bTravelled ? 1 : 0);
    OnJoinDone.Broadcast(bTravelled);
}

// ---------------------------------------------------------------- LEAVE / DESTROY
void UOnlineSessionSubsystem::LeaveGame(bool bReturnToMenu)
{
    const EPendingSessionOp Op = bReturnToMenu ? EPendingSessionOp::LeaveToMenu : EPendingSessionOp::Leave;
    if (DestroyExistingSession(Op))
    {
        return;
    }

    // Nothing to destroy: still honor navigation and notify listeners.
    if (bReturnToMenu)
    {
        TravelToMainMenu();
    }
    OnLeaveGameDone.Broadcast();
}

void UOnlineSessionSubsystem::CleanupLingeringSession()
{
    // Only destroys if a session actually exists; no travel, no broadcast otherwise.
    DestroyExistingSession(EPendingSessionOp::Leave);
}

bool UOnlineSessionSubsystem::DestroyExistingSession(EPendingSessionOp NextOp)
{
    if (!SessionInterface.IsValid() || !SessionInterface->GetNamedSession(SESSION_NAME))
    {
        return false;
    }

    PendingOp = NextOp;
    DestroySessionHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
        FOnDestroySessionCompleteDelegate::CreateUObject(
            this, &UOnlineSessionSubsystem::HandleDestroySessionComplete));

    if (SessionInterface->DestroySession(SESSION_NAME))
    {
        return true; // continuation happens in HandleDestroySessionComplete
    }

    // The destroy could not even start: undo and let the caller run NextOp immediately.
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionHandle);
    PendingOp = EPendingSessionOp::None;
    return false;
}

void UOnlineSessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionHandle);
    }
    UE_LOG(LogKitchenUnderPressure, Log, TEXT("[Online] DestroySession %s"),
        bWasSuccessful ? TEXT("OK") : TEXT("FAIL"));

    const EPendingSessionOp Op = PendingOp;
    PendingOp = EPendingSessionOp::None;

    switch (Op)
    {
    case EPendingSessionOp::Host:        DoCreateSession(); break;
    case EPendingSessionOp::Join:        DoJoinSession();   break;
    case EPendingSessionOp::LeaveToMenu: TravelToMainMenu(); OnLeaveGameDone.Broadcast(); break;
    case EPendingSessionOp::Leave:       OnLeaveGameDone.Broadcast(); break;
    default: break;
    }
}

void UOnlineSessionSubsystem::TravelToMainMenu()
{
    const UGameMapSettings* Maps = GetDefault<UGameMapSettings>();
    UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (Maps && World)
    {
        UGameplayStatics::OpenLevel(World, FName(*Maps->GetMainMenuMapName()));
    }
}
