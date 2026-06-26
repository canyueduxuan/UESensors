// Fill out your copyright notice in the Description page of Project Settings.


#include "PointCloud2DGenerator.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

APointCloud2DGenerator::APointCloud2DGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	ScanArea = CreateDefaultSubobject<UBoxComponent>(TEXT("ScanArea"));
	RootComponent = ScanArea;
	ScanArea->SetBoxExtent(FVector(500.f, 500.f, 250.f));
}
 
void APointCloud2DGenerator::BeginPlay()
{
	Super::BeginPlay();
	TraceDelegate.BindUObject(this, &APointCloud2DGenerator::OnTraceCompleted);
}
 
void APointCloud2DGenerator::StartExport()
{
	if (CurrentState != EGeneratorState::Idle && CurrentState != EGeneratorState::Completed) return;
 
	FVector Extent = ScanArea->GetScaledBoxExtent();
	FVector Center = ScanArea->GetComponentLocation();
	float ResInCm = Resolution * 100.f;
 
	GridWidth = FMath::CeilToInt((Extent.X * 2.f) / ResInCm);
	GridHeight = FMath::CeilToInt((Extent.Y * 2.f) / ResInCm);
 
	if (GridWidth <= 0 || GridHeight <= 0) return;
 
	CurrentState = EGeneratorState::Exporting;
	GridData.Empty();
	GridData.AddDefaulted(GridWidth * GridHeight);
	
	TotalTracesNeeded = GridWidth * GridHeight;
	TracesReceived = 0;
 
	// 从Box左上角开始扫描
	FVector StartCorner = Center - GetActorForwardVector() * Extent.X - GetActorRightVector() * Extent.Y + GetActorUpVector() * Extent.Z;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
 
	for (int32 y = 0; y < GridHeight; ++y)
	{
		for (int32 x = 0; x < GridWidth; ++x)
		{
			FVector RayStart = StartCorner 
				+ GetActorForwardVector() * (x * ResInCm)
				+ GetActorRightVector() * (y * ResInCm);
			FVector RayEnd = RayStart - GetActorUpVector() * (Extent.Z * 2.f);
 
			// 执行异步射线，利用UserData存储索引
			GetWorld()->AsyncLineTraceByChannel(EAsyncTraceType::Single, RayStart, RayEnd, TerrainChannel, Params, 
				FCollisionResponseParams::DefaultResponseParam, &TraceDelegate, (uint32)(y * GridWidth + x));
		}
	}
}
 
void APointCloud2DGenerator::OnTraceCompleted(const FTraceHandle& TraceHandle, FTraceDatum& TraceDatum)
{
	int32 Index = (int32)TraceDatum.UserData;
	if (TraceDatum.OutHits.Num() > 0)
	{
		GridData[Index].bHasTerrain = true;
		GridData[Index].HitLocation = TraceDatum.OutHits[0].ImpactPoint;
	}
 
	TracesReceived++;
	if (TracesReceived >= TotalTracesNeeded)
	{
		RunOverlapChecks();
	}
}
 
void APointCloud2DGenerator::RunOverlapChecks()
{
	CurrentState = EGeneratorState::Processing;
	FVector OverlapHalfExtent(Resolution * 50, Resolution * 50, VehicleHeight * 50); // 0.2x0.2x0.5m 的一半
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
 
	for (int32 i = 0; i < GridData.Num(); ++i)
	{
		if (GridData[i].bHasTerrain)
		{
			// 在命中点上方0.25m处放置中心，检测高度为0.5m的范围
			FVector CheckPos = GridData[i].HitLocation + FVector(0, 0, VehicleHeight * 50);
			TArray<FOverlapResult> Overlaps;
			GridData[i].bHasObstacle = GetWorld()->OverlapMultiByChannel(Overlaps, CheckPos, FQuat::Identity, ObstacleChannel, FCollisionShape::MakeBox(OverlapHalfExtent), Params);
		}
	}
	SaveResultToPNG();
}
 
void APointCloud2DGenerator::SaveResultToPNG()
{
	CurrentState = EGeneratorState::SavingImage;
	TArray<uint8> Pixels;
	Pixels.AddUninitialized(GridWidth * GridHeight);
 
	for (int32 i = 0; i < GridData.Num(); ++i)
	{
		Pixels[i] = GridData[i].bHasObstacle ? 0 : 255;
		// if (!GridData[i].bHasTerrain) Pixels[i] = FColor::Black;
		// else Pixels[i] = GridData[i].bHasObstacle ? FColor::Red : FColor::White;
	}
 
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
 
	if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(Pixels.GetData(), Pixels.GetAllocatedSize(), GridWidth, GridHeight, ERGBFormat::Gray, 8))
	{
		FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(), *ExportFile);
	}
	CurrentState = EGeneratorState::Completed;
}