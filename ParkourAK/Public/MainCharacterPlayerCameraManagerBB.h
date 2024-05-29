// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "MainCharacterPlayerCameraManagerBB.generated.h"

/**
 * 
 */
UCLASS()
class PARKOURAK_API AMainCharacterPlayerCameraManagerBB : public APlayerCameraManager
{
private:
	GENERATED_BODY()

	//Variables
	UPROPERTY(EditDefaultsOnly) float CrouchBlendDuration = 0.5f;
	float CrouchBlendTime;
public:
	AMainCharacterPlayerCameraManagerBB();

	//virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;

};
