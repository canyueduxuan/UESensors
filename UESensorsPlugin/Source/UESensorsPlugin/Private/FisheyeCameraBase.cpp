// Fill out your copyright notice in the Description page of Project Settings.


#include "FisheyeCameraBase.h"

// Sets default values
AFisheyeCameraBase::AFisheyeCameraBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CaptureComponent = CreateDefaultSubobject<USceneCaptureComponentCube>(TEXT("CaptureComponent"));
	CaptureComponent->bCaptureRotation = true;
    CaptureComponent->MaxViewDistanceOverride = 2000.0f;
    CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;
	RootComponent = CaptureComponent;

}

// Called when the game starts or when spawned
void AFisheyeCameraBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (!RenderTargetCube)
    {
        RenderTargetCube = NewObject<UTextureRenderTargetCube>(this);
        RenderTargetCube->Init(1024, PF_FloatRGBA);
		RenderTargetCube->UpdateResource();
    }

	CaptureComponent->TextureTarget = RenderTargetCube;

	if (FisheyeMaterialBase)
    {
        FisheyeMaterialInstance = UMaterialInstanceDynamic::Create(FisheyeMaterialBase, this);
        FisheyeMaterialInstance->SetTextureParameterValue(FName("ParamCube"), RenderTargetCube);
        UpdateMaterialParameters();
    }
}

// Called every frame
void AFisheyeCameraBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#if WITH_EDITOR
void AFisheyeCameraBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    UpdateMaterialParameters();
}
#endif

void AFisheyeCameraBase::UpdateMaterialParameters()
{
    if (!FisheyeMaterialInstance) return;

    FisheyeMaterialInstance->SetScalarParameterValue(FName("fx"), fx);
    FisheyeMaterialInstance->SetScalarParameterValue(FName("fy"), fy);
    FisheyeMaterialInstance->SetScalarParameterValue(FName("cx"), cx);
    FisheyeMaterialInstance->SetScalarParameterValue(FName("cy"), cy);
    FisheyeMaterialInstance->SetScalarParameterValue(FName("resolution_x"), resolution_x);
    FisheyeMaterialInstance->SetScalarParameterValue(FName("resolution_y"), resolution_y);

    FisheyeMaterialInstance->SetScalarParameterValue(FName("ModelType"), static_cast<float>(CurrentCameraModel));

    switch (CurrentCameraModel)
    {
        case EFisheyeModel::UCM:
            FisheyeMaterialInstance->SetScalarParameterValue(FName("UCM_alpha"), UCM_alpha);
            break;

        case EFisheyeModel::EUCM:
            FisheyeMaterialInstance->SetScalarParameterValue(FName("EUCM_alpha"), EUCM_alpha);
            FisheyeMaterialInstance->SetScalarParameterValue(FName("EUCM_beta"), EUCM_beta);
            break;

        case EFisheyeModel::DoubleSphere:
            FisheyeMaterialInstance->SetScalarParameterValue(FName("DS_alpha"), DS_alpha);
            FisheyeMaterialInstance->SetScalarParameterValue(FName("DS_xi"), DS_xi);
            break;
        
        case EFisheyeModel::KB:
            for (int i = 0; i < KB_k.Num(); ++i)
            {
                FisheyeMaterialInstance->SetScalarParameterValue(FName(*FString::Printf(TEXT("KB_k%d"), i + 1)), KB_k[i]);
            }
            break;

        case EFisheyeModel::OCAM:
            for (int i = 0; i < OCAM_a.Num(); ++i)
            {
                FisheyeMaterialInstance->SetScalarParameterValue(FName(*FString::Printf(TEXT("OCAM_a%d"), i)), OCAM_a[i]);
            }
            FisheyeMaterialInstance->SetScalarParameterValue(FName("OCAM_c"), OCAM_c);
            FisheyeMaterialInstance->SetScalarParameterValue(FName("OCAM_d"), OCAM_d);
            FisheyeMaterialInstance->SetScalarParameterValue(FName("OCAM_e"), OCAM_e);
            break;
    }

    FisheyeMaterialInstance->SetScalarParameterValue(FName("MaxDepth"), CaptureComponent->MaxViewDistanceOverride);
}