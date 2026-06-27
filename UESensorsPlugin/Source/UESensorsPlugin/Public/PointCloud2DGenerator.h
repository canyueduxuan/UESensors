// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "WorldCollision.h"
#include "PointCloud2DGenerator.generated.h"

UENUM(BlueprintType)
enum class EGeneratorState : uint8
{
	Idle,           // 空闲
	Exporting,      // 异步射线扫描中
	Processing,     // 执行Overlap检测
	SavingImage,    // 生成PNG
	SavingLocalData, // 生成本地地图数据
	Completed       // 已完成
};

struct FScanCellData
{
	FVector HitLocation;
	bool bHasTerrain = false;
	bool bHasObstacle = false;
};

USTRUCT(BlueprintType)
struct FLocalMapParams
{
    GENERATED_BODY()
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Local Map")
	float resolution = 0.125f; // 0.125m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Local Map")
    float sideSize = 16.f; // 16m
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Local Map")
    float forwardDistance = 32.f; // 32m
};

UCLASS()
class UESENSORSPLUGIN_API APointCloud2DGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APointCloud2DGenerator();
protected:
	virtual void BeginPlay() override;
 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
	TObjectPtr<UBoxComponent> ScanArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	bool bEnableLocalMap = true;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float Resolution = 0.2f; // 0.2m

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	float VehicleHeight = 0.5f; // 车辆高度
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	TEnumAsByte<ECollisionChannel> TerrainChannel = ECC_WorldStatic;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	TEnumAsByte<ECollisionChannel> ObstacleChannel = ECC_WorldDynamic;
 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
	EGeneratorState CurrentState = EGeneratorState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
	FLocalMapParams LocalMapParams;
 
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
    FString ExportFile = TEXT("E:\\Unreal_Projetcts\\UESensors\\Saved\\Baking\\image.png");
	/** 开启导出流程 */
	UFUNCTION(BlueprintCallable, Category = "Generator")
	void StartExport();
	UFUNCTION(BlueprintCallable, Category = "Generator")
	bool IsExporting() const { return CurrentState != EGeneratorState::Idle && CurrentState != EGeneratorState::Completed; }
	UFUNCTION(BlueprintCallable, Category = "Generator")
	bool IsReadyToSaveLocalMap() const { return CurrentState == EGeneratorState::SavingLocalData; }
	UFUNCTION(BlueprintCallable, Category = "Generator")
	void SaveLocalDataToPNG(FVector QueryLocation, FRotator QueryRotation, FString LocalMapFilePath);

private:
	/** 异步射线回调 */
	void OnTraceCompleted(const FTraceHandle& TraceHandle, FTraceDatum& TraceDatum);
	void RunOverlapChecks();
	void SaveResultToPNG();
 
	TArray<uint8> CachedPixels;
	TArray<FScanCellData> GridData;
	int32 GridWidth = 0;
	int32 GridHeight = 0;
	int32 TotalTracesNeeded = 0;
	int32 TracesReceived = 0;
 
	FTraceDelegate TraceDelegate;

};
