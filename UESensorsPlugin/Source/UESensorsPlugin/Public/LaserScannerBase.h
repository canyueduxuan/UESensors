// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LaserScannerBase.generated.h"

UCLASS()
class UESENSORSPLUGIN_API ALaserScannerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaserScannerBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LaserScanner|Components")
	UStaticMeshComponent* ScannerMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserScanner|Parameters")
	float scanHz = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserScanner|Parameters")
	float horizontalFov = 120.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserScanner|Parameters")
	float horizontalResolution = 0.625f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserScanner|Parameters")
	float verticalFov = 90.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserScanner|Parameters")
	float verticalResolution = 0.625f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserScanner|Parameters")
	float minRange = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserScanner|Parameters")
	float maxRange = 7500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LaserScanner|Debug")
	bool drawDebugLine = false;

	float accumulatedTime = 0.0f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void ScanOnce();

};
