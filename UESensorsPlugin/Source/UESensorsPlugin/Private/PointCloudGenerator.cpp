// Fill out your copyright notice in the Description page of Project Settings.


#include "PointCloudGenerator.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "CoordinateTransformation.h"

// Sets default values
APointCloudGenerator::APointCloudGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 创建可视化包围盒，方便在编辑器里看清扫描区域
    PreviewBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("PreviewBounds"));
    RootComponent = PreviewBounds;
    PreviewBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void APointCloudGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (PreviewBounds)
    {
        PreviewBounds->SetBoxExtent(FVector(Length, Width, Height) * 0.5f, true);
    }
}

// Called when the game starts or when spawned
void APointCloudGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APointCloudGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 如果处于处理状态，每帧消耗固定的时间预算进行碰撞检测
    if (bIsProcessing)
    {
        ProcessVoxelChunk();
    }

}

void APointCloudGenerator::StartExport()
{
    if (bIsProcessing) return;

    FVector ActorLocation = GetActorLocation();
    // 计算3D格网的左下角起点 (XMin, YMin, ZMin)
    GridStartPos = ActorLocation - FVector(Length, Width, Height) * 0.5f;

    // 计算三轴各自需要采样的体素数量
    CountX = FMath::CeilToInt(Length / VoxelResolution);
    CountY = FMath::CeilToInt(Width / VoxelResolution);
    CountZ = FMath::CeilToInt(Height / VoxelResolution);

    TotalVoxels = CountX * CountY * CountZ;
    CurrentIndex = 0;
    
    // 预分配内存防止动态扩容卡顿
    CapturedPoints.Empty(TotalVoxels / 20); 

    bIsProcessing = true;
    UE_LOG(LogTemp, Log, TEXT("点云扫描开始 -> 总计体素检测量: %d"), TotalVoxels);
}

void APointCloudGenerator::ProcessVoxelChunk()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 精准高精度计时器
    double FrameStartTime = FPlatformTime::Seconds();
    double TimeBudget = MaxFrameTimeMs / 1000.0; // 毫秒转秒

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this); // 忽略自身
    
    if (ActorsToIgnore.Num() > 0)
    {
        QueryParams.AddIgnoredActors(ActorsToIgnore);
    }

    // 1. 核心算法：创建与分辨率匹配的 3D 体素 Shape 
    FCollisionShape VoxelShape = FCollisionShape::MakeBox(FVector(VoxelResolution * 0.5f));

    // 2. 迭代检测循环
    while (CurrentIndex < TotalVoxels)
    {
        // 快速一维索引解算 3D 坐标索引
        int32 x = CurrentIndex / (CountY * CountZ);
        int32 Remainder = CurrentIndex % (CountY * CountZ);
        int32 y = Remainder / CountZ;
        int32 z = Remainder % CountZ;

        // 计算当前体素在世界空间中的中心点位置
        FVector VoxelCenter = GridStartPos + FVector(x * VoxelResolution, y * VoxelResolution, z * VoxelResolution) + FVector(VoxelResolution * 0.5f);

        // 3. 执行三维体素重叠检测 (Overlap 性能明显优于带有各种物理计算的 Sweep)
        // 使用 ECC_Visibility（或者根据需要改成 ECC_WorldStatic）
        bool bHasOverlap = World->OverlapAnyTestByChannel(VoxelCenter, FQuat::Identity, TraceChannel, VoxelShape, QueryParams);

        if (bHasOverlap)
        {
            CapturedPoints.Add(VoxelCenter);
        }

        CurrentIndex++;

        // 4. 防卡顿核心控制：检查当前帧耗时是否超标
        if (FPlatformTime::Seconds() - FrameStartTime >= TimeBudget)
        {
            float Progress = (float)CurrentIndex / TotalVoxels * 100.0f;
            UE_LOG(LogTemp, Log, TEXT("扫描进行中... 当前进度: %.2f%% | 已捕获点数: %d"), Progress, CapturedPoints.Num());
            return; // 结束本帧，将烂摊子留给下一帧的 Tick 接着干
        }
    }

    // 5. 循环安全退出，说明全部体素检测完毕
    bIsProcessing = false;
    UE_LOG(LogTemp, Log, TEXT("扫描圆满结束！共生成有效点数: %d。正在写入磁盘..."), CapturedPoints.Num());
    SaveToPLY();
}

void APointCloudGenerator::SaveToPLY()
{
    TArray<FString> FileLines;
    
    // 写入标准 PLY ASCII 头部
    FileLines.Add(TEXT("ply"));
    FileLines.Add(TEXT("format ascii 1.0"));
    FileLines.Add(FString::Printf(TEXT("element vertex %d"), CapturedPoints.Num()));
    FileLines.Add(TEXT("property float x"));
    FileLines.Add(TEXT("property float y"));
    FileLines.Add(TEXT("property float z"));
    FileLines.Add(TEXT("end_header"));

    // 写入点云数据
    for (const FVector& Point : CapturedPoints)
    {
        // 使用坐标转换函数将 UE5 坐标转换为 ROS 坐标
        FVector ROSPoint = UCoordinateTransformation::ConvertPositionUEToROS(Point);

        FileLines.Add(FString::Printf(TEXT("%.4f %.4f %.4f"), ROSPoint.X, ROSPoint.Y, ROSPoint.Z));
    }

    // 保存文件
    if (FFileHelper::SaveStringArrayToFile(FileLines, *ExportFile))
    {
        UE_LOG(LogTemp, Log, TEXT("【成功】点云已完美导出至: %s"), *ExportFile);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("【失败】点云文件写入磁盘失败！"));
    }
}
