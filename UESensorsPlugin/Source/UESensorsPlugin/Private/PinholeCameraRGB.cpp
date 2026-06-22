// Fill out your copyright notice in the Description page of Project Settings.


#include "PinholeCameraRGB.h"
#include "Kismet/KismetRenderingLibrary.h"

// Sets default values
APinholeCameraRGB::APinholeCameraRGB()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
	RootComponent = CaptureComponent;

	static FSoftObjectPath MaterialPath(TEXT("/UESensorsPlugin/CameraSensors/Materials/M_PinholeRGB.M_PinholeRGB"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(*MaterialPath.ToString());

    // 2. 检查是否找到并赋值
    if (MaterialFinder.Succeeded())
    {
        MaterialBase = MaterialFinder.Object;
    }
}

// Called when the game starts or when spawned
void APinholeCameraRGB::BeginPlay()
{
	Super::BeginPlay();

	if (!RenderTarget2D)
    {
        RenderTarget2D = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), resolution_x, resolution_y, ETextureRenderTargetFormat::RTF_RGBA8);
		RenderTarget2D->UpdateResource();
    }

	CaptureComponent->TextureTarget = RenderTarget2D;

	if (MaterialBase)
    {
        MaterialInstance = UMaterialInstanceDynamic::Create(MaterialBase, this);
        MaterialInstance->SetTextureParameterValue(FName("Param2D"), RenderTarget2D);
        // UpdateMaterialParameters();
    }
	
}

// Called every frame
void APinholeCameraRGB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UMaterialInterface* APinholeCameraRGB::GetCameraMaterial_Implementation()
{
    return MaterialInstance;
}

void APinholeCameraRGB::SavePinholeImageToDisk(const FString& FilePath)
{
	if (!MaterialInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("MaterialInstance is null. Cannot save image."));
        return;
    }

    if (!RenderTarget2DSave)
	{
		RenderTarget2DSave = NewObject<UTextureRenderTarget2D>(this);
		RenderTarget2DSave->InitCustomFormat(resolution_x, resolution_y, PF_B8G8R8A8, false);
		RenderTarget2DSave->UpdateResource();
	}

    UKismetRenderingLibrary::ClearRenderTarget2D(GetWorld(), RenderTarget2DSave, FLinearColor::Black);

    UCanvas* Canvas = nullptr;
    FVector2D Size;
    FDrawToRenderTargetContext Context;

    UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), RenderTarget2DSave, Canvas, Size, Context);

    if (Canvas && MaterialInstance)
    {
        FCanvasTileItem TileItem(FVector2D(0.f, 0.f), MaterialInstance->GetRenderProxy(), Size);
        TileItem.BlendMode = SE_BLEND_Opaque; // 强行使用不透明渲染，填充 Alpha 通道
        Canvas->DrawItem(TileItem);
    }

     UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), Context);

     FlushRenderingCommands();

     FTextureRenderTargetResource* RTResource = RenderTarget2DSave->GameThread_GetRenderTargetResource();
    if (!RTResource)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get RenderTargetResource."));
        return;
    }

    TArray<FColor> OutPixels;
    if (!RTResource->ReadPixels(OutPixels))
    {
        UE_LOG(LogTemp, Error, TEXT("ReadPixels failed! Can't read GPU texture data."));
        return;
    }

    for (FColor& Pixel : OutPixels)
    {
        Pixel.A = 255; 
    }

    TArray<uint8> CompressedPngData;
    FImageUtils::CompressImageArray(resolution_x, resolution_y, OutPixels, CompressedPngData);

    if (FFileHelper::SaveArrayToFile(CompressedPngData, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully generated and saved standard PNG: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save PNG file to disk. Check path permissions: %s"), *FilePath);
    }
}
