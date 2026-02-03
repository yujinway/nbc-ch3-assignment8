#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "WaveDataRow.h"
#include "SpartaGameState.generated.h"

UCLASS()
class SPARTAPROJECT_API ASpartaGameState : public AGameState
{
	GENERATED_BODY()

public:
	ASpartaGameState();
    
	virtual void BeginPlay() override;
    
	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category = "Score")
	int32 Score;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 SpawnedCoinCount;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Coin")
	int32 CollectedCoinCount;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 CurrentLevelIndex;
    
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level")
	int32 MaxLevels;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level")
	TArray<FName> LevelMapNames;
    
	// 웨이브 관련 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 CurrentWaveIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	int32 MaxWavesPerLevel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	UDataTable* WaveDataTable;  // 모든 레벨에서 공통 사용

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<UDataTable*> ItemSpawnTables;  // 레벨별 아이템 스폰 확률 테이블
	
	FTimerHandle WaveTimerHandle;
	FTimerHandle HUDUpdateTimerHandle;
    
	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScore() const;
    
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Amount);
    
	UFUNCTION(BlueprintCallable, Category = "Level")
	void OnGameOver();
    
	void StartLevel();
	void StartWave();
	void OnWaveTimeUp();
	void OnCoinCollected();
	void EndWave();
	void EndLevel();
	void UpdateHUD();
};