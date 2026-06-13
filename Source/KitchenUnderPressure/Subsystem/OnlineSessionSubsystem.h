// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSubsystem.generated.h"

// Neutral struct exposed to the UI: never leaks any EOS type.
USTRUCT(BlueprintType)
struct FGameSessionInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FString OwnerName;
    UPROPERTY(BlueprintReadOnly) int32 OpenSlots = 0;
    UPROPERTY(BlueprintReadOnly) int32 MaxSlots = 0;
    UPROPERTY(BlueprintReadOnly) int32 PingMs = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginDone, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostDone, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindDone, int32, NumResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinDone, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeaveDone);

/**
 * Wraps the EOS online session lifecycle (login, host, find, join, leave) behind a neutral
 * interface. No EOS type leaks out; the UI only ever sees FGameSessionInfo and the delegates.
 *
 * The lifecycle is symmetric and sequenced: any leftover session is destroyed (and the
 * destruction awaited) before a new host/join, and a leave waits for the destruction to
 * complete before traveling. This is what prevents EOS from keeping us registered, which
 * otherwise surfaces as "already in session" on the next join.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UOnlineSessionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Neutral interface: the rest of the game only ever calls these ---
    UFUNCTION(BlueprintCallable, Category = "Online")
    void Login(const FString& LoginToken);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void HostGame(const FString& MapName, int32 MaxPlayers);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void FindGames();

    UFUNCTION(BlueprintCallable, Category = "Online")
    void JoinGame(int32 SearchResultIndex);

    // Destroy the current session, then optionally travel back to the main menu once it is
    // actually gone (waits for the async destruction so EOS releases the registration).
    UFUNCTION(BlueprintCallable, Category = "Online")
    void LeaveGame(bool bReturnToMenu = false);

    // Defensive, side-effect free cleanup: drop a session left behind by a crash, hard quit
    // or closed host. Call this when (re)entering the main menu.
    void CleanupLingeringSession();

    UFUNCTION(BlueprintCallable, Category = "Online")
    bool IsLoggedIn() const { return bIsLoggedIn; }

    // Neutral list of found games, for the UI.
    UFUNCTION(BlueprintCallable, Category = "Online")
    const TArray<FGameSessionInfo>& GetFoundSessions() const { return FoundSessions; }

    // --- Events for the UI ---
    UPROPERTY(BlueprintAssignable) FOnLoginDone OnLoginDone;
    UPROPERTY(BlueprintAssignable) FOnHostDone OnHostDone;
    UPROPERTY(BlueprintAssignable) FOnFindDone OnFindDone;
    UPROPERTY(BlueprintAssignable) FOnJoinDone OnJoinDone;
    UPROPERTY(BlueprintAssignable) FOnLeaveDone OnLeaveGameDone;

private:
    // What to run once a pending session destruction completes.
    enum class EPendingSessionOp : uint8 { None, Host, Join, Leave, LeaveToMenu };

    // --- All EOS lives here ---
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<class FOnlineSessionSearch> SearchSettings;
    bool bIsLoggedIn = false;

    // Pending operation state, used to sequence destroy -> host/join/leave.
    EPendingSessionOp PendingOp = EPendingSessionOp::None;
    FString PendingMapName;
    int32 PendingMaxPlayers = 0;
    int32 PendingJoinIndex = INDEX_NONE;

    UPROPERTY()
    TArray<FGameSessionInfo> FoundSessions;

    // Internal EOS callbacks.
    void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful,
        const FUniqueNetId& UserId, const FString& Error);
    void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void HandleFindSessionsComplete(bool bWasSuccessful);
    void HandleJoinSessionComplete(FName SessionName,
        EOnJoinSessionCompleteResult::Type Result);
    void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);

    // Actual create/join, run either directly or once a pending destroy completes.
    void DoCreateSession();
    void DoJoinSession();

    // If a session is currently registered, start destroying it and remember NextOp to run
    // when the destruction completes. Returns true if a destruction was actually started.
    bool DestroyExistingSession(EPendingSessionOp NextOp);

    void TravelToMainMenu();

    // Delegate handles (for clean removal).
    FDelegateHandle CreateSessionHandle;
    FDelegateHandle FindSessionsHandle;
    FDelegateHandle JoinSessionHandle;
    FDelegateHandle DestroySessionHandle;
};
