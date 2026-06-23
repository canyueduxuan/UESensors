// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoordinateTransformation.generated.h"

/**
 * 
 */
UCLASS()
class UESENSORSPLUGIN_API UCoordinateTransformation : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	public:
    /**
     * 将 UE5 的位置/点(厘米) 转换为 ROS 的位置(米)
     */
    UFUNCTION(BlueprintPure, Category = "ROS Conversion")
    static FVector ConvertPositionUEToROS(const FVector& InUEPos);

    /**
     * 将 UE5 的四元数 转换为 ROS 的四元数
     */
    UFUNCTION(BlueprintPure, Category = "ROS Conversion")
    static FQuat ConvertQuatUEToROS(const FQuat& InUEQuat);

    /**
     * 将 UE5 的角速度(度/秒 或 弧度/秒) 转换为 ROS 的角速度
     * @param bInRadians 如果输入的UE5角速度已经是弧度/秒则传true；如果是编辑器默认的度/秒则传false
     */
    UFUNCTION(BlueprintPure, Category = "ROS Conversion")
    static FVector ConvertAngularVelocityUEToROS(const FVector& InUEAngVel, bool bInRadians = false);
};
