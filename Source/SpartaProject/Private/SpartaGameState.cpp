#include "SpartaGameState.h"
#include "SpartaGameInstance.h"
#include "SpartaPlayerController.h"
#include "SpawnVolume.h"
#include "CoinItem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"

ASpartaGameState::ASpartaGameState()
{
    Score = 0;
    SpawnedCoinCount = 0;
    CollectedCoinCount = 0;
    CurrentLevelIndex = 0;
    MaxLevels = 3;
    CurrentWaveIndex = 0;
    MaxWavesPerLevel = 3;
}

void ASpartaGameState::BeginPlay()
{
    Super::BeginPlay();
    
    StartLevel();
    
    GetWorldTimerManager().SetTimer(
        HUDUpdateTimerHandle,
        this,
        &ASpartaGameState::UpdateHUD,
        0.1f,
        true,
        0.2f
    );
}

int32 ASpartaGameState::GetScore() const
{
    return Score;
}

void ASpartaGameState::AddScore(int32 Amount)
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
        if (SpartaGameInstance)
        {
            SpartaGameInstance->AddToScore(Amount);
        }
    }
}

void ASpartaGameState::StartLevel()
{
    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
        {
            SpartaPlayerController->ShowGameHUD();
        }
    }
    
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
        if (SpartaGameInstance)
        {
            CurrentLevelIndex = SpartaGameInstance->CurrentLevelIndex;
        }
    }
    
    CurrentWaveIndex = 0;
    StartWave();
}

void ASpartaGameState::StartWave()
{
    UE_LOG(LogTemp, Warning, TEXT("Wave %d 시작!"), CurrentWaveIndex + 1);
    
    SpawnedCoinCount = 0;
    CollectedCoinCount = 0;
    
    // 웨이브 데이터 테이블 확인
    if (!WaveDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("웨이브 데이터 테이블이 null입니다!"));
        return;
    }
    
    // 웨이브 데이터 가져오기
    FString RowName = FString::Printf(TEXT("Wave%d"), CurrentWaveIndex + 1);
    FWaveDataRow* WaveData = WaveDataTable->FindRow<FWaveDataRow>(FName(*RowName), TEXT("WaveDataContext"));
    
    if (!WaveData)
    {
        UE_LOG(LogTemp, Error, TEXT("웨이브 데이터를 찾을 수 없습니다: %s"), *RowName);
        return;
    }
    
    // SpawnVolume 찾기
    TArray<AActor*> FoundVolumes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), FoundVolumes);
    
    if (FoundVolumes.Num() > 0)
    {
        ASpawnVolume* SpawnVolume = Cast<ASpawnVolume>(FoundVolumes[0]);
        if (SpawnVolume)
        {
            // 레벨별 아이템 스폰 테이블 설정
            if (ItemSpawnTables.IsValidIndex(CurrentLevelIndex))
            {
                SpawnVolume->ItemDataTable = ItemSpawnTables[CurrentLevelIndex];
            }
            
            // 웨이브별 아이템 개수만큼 스폰
            for (int32 i = 0; i < WaveData->ItemCount; i++)
            {
                AActor* SpawnedActor = SpawnVolume->SpawnRandomItem();
                if (SpawnedActor && SpawnedActor->IsA(ACoinItem::StaticClass()))
                {
                    SpawnedCoinCount++;
                }
            }
        }
    }
    
    // 웨이브 타이머 설정
    GetWorldTimerManager().SetTimer(
        WaveTimerHandle,
        this,
        &ASpartaGameState::OnWaveTimeUp,
        WaveData->WaveDuration,
        false
    );
}

void ASpartaGameState::OnWaveTimeUp()
{
    EndWave();
}

void ASpartaGameState::OnCoinCollected()
{
    CollectedCoinCount++;
    
    if (SpawnedCoinCount > 0 && CollectedCoinCount >= SpawnedCoinCount)
    {
        EndWave();
    }
}

void ASpartaGameState::EndWave()
{
    GetWorldTimerManager().ClearTimer(WaveTimerHandle);
    
    UE_LOG(LogTemp, Warning, TEXT("Wave %d 종료!"), CurrentWaveIndex + 1);
    
    CurrentWaveIndex++;
    
    // 다음 웨이브가 있으면 시작
    if (CurrentWaveIndex < MaxWavesPerLevel)
    {
        StartWave();
    }
    else
    {
        // 레벨의 모든 웨이브 완료
        EndLevel();
    }
}

void ASpartaGameState::EndLevel()
{
    GetWorldTimerManager().ClearTimer(HUDUpdateTimerHandle);
    GetWorldTimerManager().ClearTimer(WaveTimerHandle);
    
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
        if (SpartaGameInstance)
        {
            AddScore(Score);
            CurrentLevelIndex++;
            SpartaGameInstance->CurrentLevelIndex = CurrentLevelIndex;
            
            if (CurrentLevelIndex >= MaxLevels)
            {
                OnGameOver();
                return;
            }
            
            if (LevelMapNames.IsValidIndex(CurrentLevelIndex))
            {
                FTimerHandle TransitionTimer;
                GetWorldTimerManager().SetTimer(
                    TransitionTimer,
                    [this]()
                    {
                        if (GetWorld() && LevelMapNames.IsValidIndex(CurrentLevelIndex))
                        {
                            UGameplayStatics::OpenLevel(GetWorld(), LevelMapNames[CurrentLevelIndex]);
                        }
                    },
                    0.3f,
                    false
                );
            }
            else
            {
                OnGameOver();
            }
        }
    }
}

void ASpartaGameState::OnGameOver()
{
    GetWorldTimerManager().ClearTimer(WaveTimerHandle);
    GetWorldTimerManager().ClearTimer(HUDUpdateTimerHandle);
    
    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        if (ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController))
        {
            SpartaPlayerController->SetPause(true);
            SpartaPlayerController->ShowMainMenu(true);
        }
    }
}

void ASpartaGameState::UpdateHUD()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        ASpartaPlayerController* SpartaPlayerController = Cast<ASpartaPlayerController>(PlayerController);
        if (SpartaPlayerController)
        {
            UUserWidget* HUDWidget = SpartaPlayerController->GetHUDWidget();
            if (HUDWidget)
            {
                if (UTextBlock* TimeText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Time"))))
                {
                    float RemainingTime = GetWorldTimerManager().GetTimerRemaining(WaveTimerHandle);
                    if (RemainingTime > 0.0f)
                    {
                        TimeText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
                    }
                }
                
                if (UTextBlock* ScoreText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Score"))))
                {
                    if (UGameInstance* GameInstance = GetGameInstance())
                    {
                        USpartaGameInstance* SpartaGameInstance = Cast<USpartaGameInstance>(GameInstance);
                        if (SpartaGameInstance)
                        {
                            ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), SpartaGameInstance->TotalScore)));
                        }
                    }
                }
                
                if (UTextBlock* LevelIndexText = Cast<UTextBlock>(HUDWidget->GetWidgetFromName(TEXT("Level"))))
                {
                    int32 DisplayWaveIndex = FMath::Min(CurrentWaveIndex, MaxWavesPerLevel - 1);
                    LevelIndexText->SetText(FText::FromString(FString::Printf(TEXT("Level: %d  |  Wave: %d"), 
                        CurrentLevelIndex + 1, DisplayWaveIndex + 1)));
                }
            }
        }
    }
}