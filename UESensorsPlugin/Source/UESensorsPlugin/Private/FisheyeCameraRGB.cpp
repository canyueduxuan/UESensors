// Fill out your copyright notice in the Description page of Project Settings.


#include "FisheyeCameraRGB.h"

AFisheyeCameraRGB::AFisheyeCameraRGB()
{
    static FSoftObjectPath MaterialPath(TEXT("/UESensorsPlugin/CameraSensors/Materials/M_FisheyeRGB.M_FisheyeRGB"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(*MaterialPath.ToString());

    // 2. 检查是否找到并赋值
    if (MaterialFinder.Succeeded())
    {
        FisheyeMaterialBase = MaterialFinder.Object;
    }
}