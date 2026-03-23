
#pragma once

#include "CoreMinimal.h"
#include "TricksAnimNotifyInterface.h"
#include "Components/ActorComponent.h"
#include "Player/Tricks/TricksAnimData.h"
#include "Player/Tricks/TricksSystemData.h"
#include "TricksAnimationComponent.generated.h"

class UTricksComponent;
class AJBRCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class JUNKBOARDRUSH_API UTricksAnimationComponent : public UActorComponent, public ITricksAnimNotifyInterface
{
	GENERATED_BODY()

public:
	
	UTricksAnimationComponent();

	AJBRCharacter* Character;
	UTricksComponent* TricksComponent;

#pragma region Animations Base Functions

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayTrickAnimation(UAnimationAsset* AnimToPlayBoard, UAnimationAsset* AnimToPlayPlayer,
		const FTricksAnimDataBoard& NewBoardStructDataAnim,
		const FTricksAnimData& BoardTrickData, const FTricksAnimData& PlayerTrickData);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayInterruptAnimation(UAnimationAsset* AnimToPlayBoard, UAnimationAsset* AnimToPlayPlayer,
	const FTricksAnimDataBoard& NewBoardStructDataAnim,
	   const FTricksAnimData& BoardTrickData, const FTricksAnimData& PlayerTrickData, float BlendSpaceXvalue);

#pragma endregion Animations Base Functions

#pragma region Animations Logic

	bool StartTrickAnimation();

	UFUNCTION() void StartLandAnimation();
	void InterruptTrickWithData(FTricksAnimData BoardData, FTricksAnimData PlayerData, float BlendSpaceXvalue);
	
	// UAnimMontage ref and bool are only use for end montage event subscription
	UFUNCTION()
	void EndMainAnimationLogic(UAnimMontage* Montage = nullptr, bool bInterrupted = false);
	UFUNCTION()
	void EndPlayerAnimationLogic(UAnimMontage* Montage = nullptr, bool bInterrupted = false);
	
#pragma endregion Animations Logic

#pragma region Data

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsBoardTrickingFromAnim();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPlayerTrickingFromAnim();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool AreFootCloseEnoughToBoard(float& DistR, float& DistL);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool ShouldAttachPlayerToBoard();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool ShouldHardSetFootTransform();

	void UpdateAnimTracking(FTricksAnimData NewBoardData, FTricksAnimData NewPlayerData,
	                        FTricksAnimDataBoard NewBoardStructDataAnim, UAnimationAsset* NewBoardAnim,  UAnimationAsset* NewPlayerAnim);

	TArray<FTricksAnimDataBoard> GetCorrectBoardAnimData(FTricksAnimData BaseData);
	TArray<TObjectPtr<UAnimationAsset>> GetCorrectPlayerAnimData(FTricksAnimData BaseData);
	
	UAnimationAsset* GetCurrentBoardAnim();
	FTricksAnimDataBoard GetCurrentBoardAnimStructData();
	UAnimationAsset* GetCurrentPlayerAnim();
	FTricksAnimData GetCurrentBoardAnimData();
	FTricksAnimData GetCurrentPlayerAnimData();

	UFUNCTION(BlueprintCallable)
	const FTricksSystemData& GetTricksData() const;
	
	FAnimSelection GetAnimForBoardAndPlayer(const TArray<FTricksAnimDataBoard>& BoardStructArray,
		const TArray<TObjectPtr<UAnimationAsset>>& PlayerAnimArray);
    
#pragma endregion Data

#pragma region Data variables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tricks - Data")
	FDataTableRowHandle TricksData;

	UPROPERTY(BlueprintReadOnly, Replicated)
	UAnimationAsset* CurrentBoardTrickAnim;
	UPROPERTY(BlueprintReadOnly, Replicated)
	FTricksAnimDataBoard CurrentBoardTrickStructAnim;
	UPROPERTY(BlueprintReadOnly, Replicated)
	UAnimationAsset* CurrentPlayerTrickAnim;

	UPROPERTY(BlueprintReadOnly, Replicated)
	UAnimationAsset* LastBoardTrickAnim;
	UPROPERTY(BlueprintReadOnly, Replicated)
	FTricksAnimDataBoard LastBoardTrickStructAnim;
	UPROPERTY(BlueprintReadOnly, Replicated)
	UAnimationAsset* LastPlayerTrickAnim;

	UPROPERTY(BlueprintReadOnly, Replicated)
	FTricksAnimData CurrentBoardTrickAnimData;
	UPROPERTY(BlueprintReadOnly, Replicated)
	FTricksAnimData CurrentPlayerTrickAnimData;

	UPROPERTY(BlueprintReadOnly, Replicated)
	FTricksAnimData LastBoardTrickAnimData;
	UPROPERTY(BlueprintReadOnly, Replicated)
	FTricksAnimData LastPlayerTrickAnimData;

#pragma endregion Data variables
	
#pragma region Transition Blend
	
	
	
#pragma endregion Transition Blend

protected:

	// Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

private:
	
	UAnimInstance* PreviousBoardAnimInstance;
	UAnimInstance* PreviousPlayerAnimInstance;
	UAnimInstance* CurrentBoardAnimInstance;
	UAnimInstance* CurrentPlayerAnimInstance;
	
	UAnimMontage* PreviousBoardAnimMontage;
	UAnimMontage* PreviousPlayerAnimMontage;
	UAnimMontage* CurrentBoardAnimMontage;
	UAnimMontage* CurrentPlayerAnimMontage;
};
