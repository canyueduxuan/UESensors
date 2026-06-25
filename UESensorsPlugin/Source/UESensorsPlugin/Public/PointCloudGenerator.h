// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointCloudGenerator.generated.h"

UCLASS()
class UESENSORSPLUGIN_API APointCloudGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APointCloudGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanning Settings")
    TArray<AActor*> ActorsToIgnore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Scanner")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_Visibility; // 碰撞通道，默认使用 Visibility

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Scanner")
    float Length = 2000.f; // X轴 扫描总长度 (单位: 厘米)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Scanner")
    float Width = 2000.f;  // Y轴 扫描总宽度 (单位: 厘米)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Scanner")
    float Height = 2000.f; // Z轴 扫描总高度 (单位: 厘米)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Scanner", meta = (ClampMin = "1.0"))
    float VoxelResolution = 20.f; // 体素分辨率/采样步长 (每一个体素方块的边长)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Scanner")
    float MaxFrameTimeMs = 5.0f; // 每帧最大允许卡顿时间 (毫秒)，核心防卡顿设计

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Scanner")
    FString ExportFile = TEXT("E:\\Unreal_Projetcts\\UESensors\\Saved\\Baking\\ScenePointCloud.ply");

    // --- 控制接口 ---
    
    // 激活开始生成点云
    UFUNCTION(BlueprintCallable, Category = "Voxel Scanner")
    void StartExport();

    bool GetStatus()
    {
        return bIsProcessing;
    }

private:
	void ProcessVoxelChunk();
    void SaveToPLY();

    // 内部状态控制
    bool bIsProcessing = false;
    FVector GridStartPos;
    int32 CountX = 0;
    int32 CountY = 0;
    int32 CountZ = 0;
    int32 TotalVoxels = 0;
    int32 CurrentIndex = 0;

    // 存储检测到的碰撞点
    TArray<FVector> CapturedPoints;

    // 用于编辑器内直观预览范围的Box
    UPROPERTY()
    class UBoxComponent* PreviewBounds;

};
