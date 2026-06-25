#include "LaserScannerBase.h"
#include "DrawDebugHelpers.h"
#include "CoordinateTransformation.h"

ALaserScannerBase::ALaserScannerBase()
{
    PrimaryActorTick.bCanEverTick = true;
    ScannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScannerMesh"));
    RootComponent = ScannerMesh;
}

void ALaserScannerBase::BeginPlay()
{
    Super::BeginPlay();

    // 1. 预计算所有射线的本地方向向量（计算一次，终身受益）
    PrecomputedLocalDirections.Empty();
    for (float ve = -verticalFov / 2; ve < verticalFov / 2; ve += verticalResolution)
    {
        float RadVe = FMath::DegreesToRadians(ve);
        float CosVe = FMath::Cos(RadVe);
        float SinVe = FMath::Sin(RadVe);

        for (float ho = -horizontalFov / 2; ho < horizontalFov / 2; ho += horizontalResolution)
        {
            float RadHo = FMath::DegreesToRadians(ho);
            PrecomputedLocalDirections.Add(FVector(
                CosVe * FMath::Cos(RadHo),
                CosVe * FMath::Sin(RadHo),
                SinVe
            ));
        }
    }

    // 2. 绑定异步回调函数
    TraceDelegate.BindUObject(this, &ALaserScannerBase::OnTraceCompleted);
}

void ALaserScannerBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    accumulatedTime += DeltaTime;
    float Interval = 1.0f / FMath::Max(scanHz, 0.001f);

    if (accumulatedTime >= Interval)
    {
        ScanOnceAsync();
        accumulatedTime = 0.0f;
    }
}

void ALaserScannerBase::ScanOnceAsync()
{
    UWorld* World = GetWorld();
    if (!World || PrecomputedLocalDirections.Num() == 0) return;

    // 清空上一帧结果并预分配内存，减少 TArray 动态扩容开销
    ScanResults.Reset(); 
    ScanResults.Reserve(PrecomputedLocalDirections.Num());

    PendingTracesCount = PrecomputedLocalDirections.Num();

    FVector Start = GetActorLocation();
    FQuat ActorRotation = GetActorQuat(); // Quat 旋转比 Rotator 更高效

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    // 强制使用简单碰撞检测（Simple Collision），性能远高于复杂碰撞（Complex）
    Params.bTraceComplex = true; 

    // 发射异步追踪请求
    for (const FVector& LocalDir : PrecomputedLocalDirections)
    {
        FVector WorldDir = ActorRotation.RotateVector(LocalDir);
        FVector End = Start + (WorldDir * maxRange);

        World->AsyncLineTraceByChannel(
            EAsyncTraceType::Single,
            Start,
            End,
            ECC_Visibility,
            Params,
            FCollisionResponseParams::DefaultResponseParam,
            &TraceDelegate
        );
    }
}

void ALaserScannerBase::OnTraceCompleted(const FTraceHandle& TraceHandle, FTraceDatum& TraceDatum)
{
    // 该函数由引擎物理线程在处理完追踪后自动调用
    if (TraceDatum.OutHits.Num() > 0)
    {
        const FHitResult& Hit = TraceDatum.OutHits[0];
        if (Hit.bBlockingHit && Hit.Distance >= minRange)
        {
            // 将结果存入成员变量
            ScanResults.Add(Hit.ImpactPoint);

            if (drawDebugPoint)
            {
                DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 4.0f, FColor::Red, false, 0.1f);
            }
        }
    }
    PendingTracesCount--;
    if (PendingTracesCount == 0)
    {
        OnScanFinished.Broadcast(ScanResults);
    }
}

TArray<FVector> ALaserScannerBase::SyncScanBlocking()
{
	UWorld* World = GetWorld();
    // 1. 安全校验提前，若不满足条件直接返回空数组，避免后续访问空指针崩溃
    if (!World || PrecomputedLocalDirections.Num() == 0) 
    {
        return TArray<FVector>();
    }

	// 2. 执行阻塞追踪
	FVector Start = GetActorLocation();
	FQuat ActorRotation = GetActorQuat();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = true;

	TArray<FVector> LocalSyncResults;
    LocalSyncResults.Reserve(PrecomputedLocalDirections.Num());

	for (const FVector& LocalDir : PrecomputedLocalDirections)
	{
		FVector WorldDir = ActorRotation.RotateVector(LocalDir);
		FVector End = Start + (WorldDir * maxRange);

		FHitResult Hit;
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

		if (bHit && Hit.Distance >= minRange)
		{
			LocalSyncResults.Add(Hit.ImpactPoint);

			if (drawDebugPoint)
			{
				DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 4.0f, FColor::Blue, false, 0.1f);
			}
		}
	}
 
    return LocalSyncResults;
}

void ALaserScannerBase::SaveToPLY(TArray<FVector> PointCloud, FString ExportFile)
{
    TArray<FString> FileLines;
    
    // 写入标准 PLY ASCII 头部
    FileLines.Add(TEXT("ply"));
    FileLines.Add(TEXT("format ascii 1.0"));
    FileLines.Add(FString::Printf(TEXT("element vertex %d"), PointCloud.Num()));
    FileLines.Add(TEXT("property float x"));
    FileLines.Add(TEXT("property float y"));
    FileLines.Add(TEXT("property float z"));
    FileLines.Add(TEXT("end_header"));

    for (const FVector& Point : PointCloud)
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