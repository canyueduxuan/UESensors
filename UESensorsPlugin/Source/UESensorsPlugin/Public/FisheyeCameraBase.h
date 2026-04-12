// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponentCube.h"
#include "Engine/TextureRenderTargetCube.h"
#include "TextureVisualizeInterface.h"
#include "FisheyeCameraBase.generated.h"

UENUM(BlueprintType)
enum class EFisheyeModel : uint8
{
    UCM  UMETA(DisplayName = "UCM (Unified Camera Model)"),
    EUCM UMETA(DisplayName = "EUCM (Enhanced Unified Camera Model)"),
    DoubleSphere UMETA(DisplayName = "DS (Double Sphere)"),
	KB UMETA(DisplayName = "KB (Kannala-Brandt)"),
	OCAM UMETA(DisplayName = "OCAM (OCamCalib Camera Model)")
};

UCLASS()
class UESENSORSPLUGIN_API AFisheyeCameraBase : public AActor, public ITextureVisualizeInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFisheyeCameraBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual UMaterialInterface* GetCameraMaterial_Implementation() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fisheye")
    USceneCaptureComponentCube* CaptureComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye")
    UTextureRenderTargetCube* RenderTargetCube;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye")
    UMaterialInterface* FisheyeMaterialBase;

	UPROPERTY(BlueprintReadWrite)
    UMaterialInstanceDynamic* FisheyeMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Model Selection")
    EFisheyeModel CurrentCameraModel = EFisheyeModel::EUCM;

	//UCM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::UCM", EditConditionHides))
    float UCM_alpha = 0.61f;

	//EUCM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::EUCM", EditConditionHides))
	float EUCM_alpha = 0.61f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::EUCM", EditConditionHides))
	float EUCM_beta = 1.07f;

	//DS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::DoubleSphere", EditConditionHides))
	float DS_alpha = 0.61f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::DoubleSphere", EditConditionHides))
	float DS_xi = 1.07f;

	//KB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::KB", EditConditionHides))
	TArray<float> KB_k = {-0.03f, 0.0f, 0.0f, 0.0f};

	//OCAM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::OCAM", EditConditionHides))
	TArray<float> OCAM_a = {-350.0f, 0.0f, 0.0011f, 0.0f, 0.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::OCAM", EditConditionHides))
	float OCAM_c = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::OCAM", EditConditionHides))
	float OCAM_d = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters", meta = (EditCondition = "CurrentCameraModel == EFisheyeModel::OCAM", EditConditionHides))
	float OCAM_e = 0.0f;

	//common parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters")
	float fx = 138.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters")
	float fy = 138.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters")
	float cx = 256.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters")
	float cy = 256.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters")
	int32 resolution_x = 512;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fisheye|Parameters")
	int32 resolution_y = 512;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UFUNCTION(BlueprintCallable, Category = "Fisheye")
    void UpdateMaterialParameters();

};
