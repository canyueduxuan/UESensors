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

void APinholeCameraRGB::SavePinholeImageToDisk(FString FilePath)
{
    if (!CaptureComponent || !RenderTarget2D || !MaterialInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Resources not ready for saving."));
        return;
    }
 
    // --- 关键点 1: 强制更新场景捕捉 ---
    CaptureComponent->CaptureScene(); // 立即捕捉当前画面到 RenderTarget2D
 
    // --- 关键点 2: 确保渲染完成 ---
    // 强制渲染线程完成捕捉任务
    FlushRenderingCommands();
 
    if (!RenderTarget2DSave)
    {
        RenderTarget2DSave = NewObject<UTextureRenderTarget2D>(this);
        RenderTarget2DSave->InitCustomFormat(resolution_x, resolution_y, PF_B8G8R8A8, false);
    }
    // 每次更新尺寸（如果可能变动）
    RenderTarget2DSave->UpdateResource();
 
    // 绘制到中间 RT (应用材质，如你的 Pinhole 畸变)
    FDrawToRenderTargetContext Context;
    UCanvas* Canvas = nullptr;
    FVector2D Size;
    UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(GetWorld(), RenderTarget2DSave, Canvas, Size, Context);
    if (Canvas)
    {
        // 确保材质参数已链接到最新的 RenderTarget2D
        MaterialInstance->SetTextureParameterValue(FName("Param2D"), RenderTarget2D);
        FCanvasTileItem TileItem(FVector2D(0.f, 0.f), MaterialInstance->GetRenderProxy(), Size);
        TileItem.BlendMode = SE_BLEND_Opaque;
        Canvas->DrawItem(TileItem);
    }
    UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(GetWorld(), Context);
 
    // 再次刷新，确保 Canvas 绘制完成
    FlushRenderingCommands();
 
    // --- 关键点 3: 读取像素并保存 ---
    FTextureRenderTargetResource* RTResource = RenderTarget2DSave->GameThread_GetRenderTargetResource();
    TArray<FColor> OutPixels;
    if (RTResource && RTResource->ReadPixels(OutPixels))
    {
        // 修正 Alpha 通道
        for (FColor& Pixel : OutPixels) { Pixel.A = 255; }
 
        // --- 关键点 4: 自动生成唯一文件名 (防止重复保存同一个文件名) ---
        if (FilePath.IsEmpty() || FPaths::FileExists(FilePath))
        {
            FString Directory = FPaths::ProjectSavedDir() / TEXT("Captures/");
            IFileManager::Get().MakeDirectory(*Directory);
            FilePath = Directory + TEXT("Img_") + FDateTime::Now().ToString() + TEXT(".png");
        }
 
        TArray<uint8> CompressedPngData;
        FImageUtils::CompressImageArray(resolution_x, resolution_y, OutPixels, CompressedPngData);
        if (FFileHelper::SaveArrayToFile(CompressedPngData, *FilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Saved Unique Image: %s"), *FilePath);
        }
    }
}
