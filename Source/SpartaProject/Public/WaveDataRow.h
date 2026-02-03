#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WaveDataRow.generated.h"

USTRUCT(BlueprintType)
struct FWaveDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WaveNumber;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WaveDuration;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemCount;
};