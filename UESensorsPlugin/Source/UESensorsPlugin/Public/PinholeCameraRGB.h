// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "PinholeCameraRGB.generated.h"

UCLASS()
class UESENSORSPLUGIN_API APinholeCameraRGB : public AActor, public ITextureVisualizeInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APinholeCameraRGB();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual UMaterialInterface* GetCameraMaterial_Implementation() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pinhole")
    USceneCaptureComponent2D* CaptureComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pinhole")
	UTextureRenderTarget2D* RenderTarget2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pinhole")
	UTextureRenderTarget2D* RenderTarget2DSave;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinhole")
    UMaterialInterface* MaterialBase;

	UPROPERTY(BlueprintReadWrite)
    UMaterialInstanceDynamic* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinhole|Parameters")
	int32 resolution_x = 512;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pinhole|Parameters")
	int32 resolution_y = 512;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Pinhole")
	void SavePinholeImageToDisk(const FString& FilePath);

};
