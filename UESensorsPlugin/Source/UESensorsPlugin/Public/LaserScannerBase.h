// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldCollision.h" // 异步追踪所需
#include "LaserScannerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScanFinished, TArray<FVector>, ScanResults);

UCLASS()
class UESENSORSPLUGIN_API ALaserScannerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaserScannerBase();
    UFUNCTION(BlueprintCallable, Category = "Scanner")
	TArray<FVector> SyncScanBlocking();
    UFUNCTION(BlueprintCallable, Category = "Scanner")
    void SaveToPLY(TArray<FVector> PointCloud, FString ExportFile);

    UPROPERTY(BlueprintAssignable)
    FOnScanFinished OnScanFinished;

    TArray<FVector> GetLastScanResults() const { return ScanResults; }

    int32 PendingTracesCount = 0;

    /** 执行异步扫描 */
    void ScanOnceAsync();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scanner")
    UStaticMeshComponent* ScannerMesh;
 
    /** 扫描参数 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner|Params")
    float scanHz = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner|Params")
    float horizontalFov = 120.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner|Params")
    float horizontalResolution = 0.625f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner|Params")
    float verticalFov = 90.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner|Params")
    float verticalResolution = 0.625f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner|Params")
    float minRange = 10.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner|Params")
    float maxRange = 7500.0f;
 
    /** 结果存储：点云数据 */
    UPROPERTY(BlueprintReadOnly, Category = "Scanner|Results")
    TArray<FVector> ScanResults;
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanner|Debug")
    bool drawDebugPoint = false;
 
private:
    /** 缓存本地空间射线方向，避免重复三角计算 */
    TArray<FVector> PrecomputedLocalDirections;
    
    float accumulatedTime = 0.0f;
    FTraceDelegate TraceDelegate;
 
    /** 异步追踪回调 */
    void OnTraceCompleted(const FTraceHandle& TraceHandle, FTraceDatum& TraceDatum);
};

