// Fill out your copyright notice in the Description page of Project Settings.


#include "CoordinateTransformation.h"

FVector UCoordinateTransformation::ConvertPositionUEToROS(const FVector& InUEPos)
{
    // X不变, Y取反, Z不变。厘米转米
    return FVector(InUEPos.X / 100.0f, -InUEPos.Y / 100.0f, InUEPos.Z / 100.0f);
}

FQuat UCoordinateTransformation::ConvertQuatUEToROS(const FQuat& InUEQuat)
{
    // 左手系四元数 -> 右手系四元数
    // 映射关系: X_ros = -X_ue; Y_ros = Y_ue; Z_ros = -Z_ue; W_ros = W_ue;
    return FQuat(-InUEQuat.X, InUEQuat.Y, -InUEQuat.Z, InUEQuat.W);
}

FVector UCoordinateTransformation::ConvertAngularVelocityUEToROS(const FVector& InUEAngVel, bool bInRadians)
{
    // 1. 如果输入是度/秒(°/s)，先转为弧度/秒(rad/s)
    FVector AngVelRad = bInRadians ? InUEAngVel : FMath::DegreesToRadians(InUEAngVel);

    // UE5: X = Roll, Y = Pitch, Z = Yaw
    float UERoll  = AngVelRad.X;
    float UEPitch = AngVelRad.Y;
    float UEYaw   = AngVelRad.Z;

    // 2. 映射到 ROS 的右手系标准 (输出向量的 X=Roll, Y=Pitch, Z=Yaw)
    float ROSRoll  = UERoll;    // 符号不变
    float ROSPitch = -UEPitch;  // 镜像取反
    float ROSYaw   = -UEYaw;    // 镜像取反

    return FVector(ROSRoll, ROSPitch, ROSYaw);
}