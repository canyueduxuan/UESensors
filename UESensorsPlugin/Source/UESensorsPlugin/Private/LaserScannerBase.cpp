// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserScannerBase.h"

// Sets default values
ALaserScannerBase::ALaserScannerBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ScannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScannerMesh"));
	RootComponent = ScannerMesh;

}

// Called when the game starts or when spawned
void ALaserScannerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALaserScannerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	accumulatedTime += DeltaTime;

    if (accumulatedTime >= 1.0f / scanHz)
    {
        ScanOnce();
        accumulatedTime -= 1.0f / scanHz; 
    }
}

void ALaserScannerBase::ScanOnce()
{
	for(float ve = -verticalFov / 2;ve < verticalFov / 2;ve += verticalResolution)
		for(float ho = -horizontalFov / 2;ho < horizontalFov / 2;ho += horizontalResolution)
		{
			float x = FMath::Cos(FMath::DegreesToRadians(ve)) * FMath::Cos(FMath::DegreesToRadians(ho));
			float y = FMath::Cos(FMath::DegreesToRadians(ve)) * FMath::Sin(FMath::DegreesToRadians(ho));
			float z = FMath::Sin(FMath::DegreesToRadians(ve));

			FVector localDirection = FVector(x, y, z);
			FVector direction = GetActorRotation().RotateVector(localDirection);

			FHitResult hitResult;
			FVector startLocation = GetActorLocation();
			FVector endLocation = startLocation + direction * maxRange;

			FCollisionQueryParams queryParams;
			queryParams.AddIgnoredActor(this);

			bool bHit = GetWorld()->LineTraceSingleByChannel(hitResult, startLocation, endLocation, ECC_Visibility, queryParams);

			if (bHit && hitResult.Distance >= minRange)
			{
				if (drawDebugLine)
				{
					DrawDebugPoint(GetWorld(), hitResult.Location, 5.0f, FColor::Red, false, -1.0f);
					// DrawDebugLine(GetWorld(), startLocation, hitResult.Location, FColor::Green, false, -1.0f, 0, 1.0f);
				}
			}
		}
}