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
	CachedPixels.Empty();
	CachedPixels.AddUninitialized(GridWidth * GridHeight);
 
	for (int32 i = 0; i < GridData.Num(); ++i)
	{
		CachedPixels[i] = GridData[i].bHasObstacle ? 0 : 255;
	}
 
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
 
	if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(CachedPixels.GetData(), CachedPixels.GetAllocatedSize(), GridWidth, GridHeight, ERGBFormat::Gray, 8))
	{
		FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(), *ExportFile);
	}

	if (bEnableLocalMap)
	{
		CurrentState = EGeneratorState::SavingLocalData;
	}
	else
	{
		CurrentState = EGeneratorState::Completed;
	}
}

void APointCloud2DGenerator::SaveLocalDataToPNG(FVector QueryLocation, FRotator QueryRotation, FString LocalMapFilePath)
{
	if (CachedPixels.Num() == 0) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No cached pixel data available. Please run StartExport() first."));
		return;
	}
	int32 result_width = FMath::CeilToInt(2.0f * LocalMapParams.sideSize / LocalMapParams.resolution);
	int32 result_height = FMath::CeilToInt(LocalMapParams.forwardDistance / LocalMapParams.resolution);

	TArray<uint8> result_pixels;
	result_pixels.AddUninitialized(result_width * result_height);

	FVector GlobalCenter = ScanArea->GetComponentLocation();
    FVector GlobalExtent = ScanArea->GetScaledBoxExtent();

	float GlobalResCm = Resolution * 100.0f;

	FTransform QueryTransform(QueryRotation, QueryLocation);

	for (int32 y = 0; y < result_height; ++y)
    {
        for (int32 x = 0; x < result_width; ++x)
		{
			float LocalX = LocalMapParams.forwardDistance * 100.f - (y * LocalMapParams.resolution * 100.f) - (LocalMapParams.resolution * 100.f * 0.5f);
			float LocalY = (x * LocalMapParams.resolution * 100.f) - (LocalMapParams.sideSize * 100.f) + (LocalMapParams.resolution * 100.f * 0.5f);

			FVector WorldPoint = QueryTransform.TransformPosition(FVector(LocalX, LocalY, 0.0f));

			int32 GlobalX = FMath::FloorToInt((WorldPoint.X - (GlobalCenter.X - GlobalExtent.X)) / GlobalResCm);
			int32 GlobalY = FMath::FloorToInt((WorldPoint.Y - (GlobalCenter.Y - GlobalExtent.Y)) / GlobalResCm);

			if (GlobalX >= 0 && GlobalX < GridWidth && GlobalY >= 0 && GlobalY < GridHeight)
			{
				int32 GlobalIndex = GlobalY * GridWidth + GlobalX;
				int32 ResultIndex = y * result_width + x;
				result_pixels[ResultIndex] = CachedPixels[GlobalIndex];
			}
			else
			{
				int32 ResultIndex = y * result_width + x;
				result_pixels[ResultIndex] = 255; // Out of bounds, mark as free space
			}
		}
	}

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	if (ImageWrapper.IsValid() && ImageWrapper->SetRaw(result_pixels.GetData(), result_pixels.GetAllocatedSize(), result_width, result_height, ERGBFormat::Gray, 8))
	{
		FFileHelper::SaveArrayToFile(ImageWrapper->GetCompressed(), *LocalMapFilePath);
	}

	CurrentState = EGeneratorState::Completed;
}