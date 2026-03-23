// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "Interfaces/OnlineSessionInterface.h"

#include "JBRGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class JUNKBOARDRUSH_API UJBRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UJBRGameInstance();
	
	UPROPERTY(BlueprintReadWrite)
	bool bCurrentSearchSessionOnSameNetwork = false;

	UFUNCTION(BlueprintCallable)
	void FindAndJoinSession(FString targetSessionCode, bool bSearchSessionOnSameNetwork);

	UFUNCTION(BlueprintCallable)
	void HostSession(FString targetSessionCode, bool bSearchSessionOnSameNetwork);
	
	UFUNCTION(BlueprintCallable)
	void LeaveGame();
	
	UPROPERTY(BlueprintReadOnly, Category = "Network")
	bool bIsNetworkBusy = false;

protected:
	virtual void Init() override;

	// Callbacks for session functions
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FString currentTargetSessionCode;
	
	bool bWantsToSearchAfterDestroy = false;
	bool bIsDestroyingSession = false;
	void InternalFindSession();
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
};
