// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacterBB.generated.h"

class UMainCharacterMovementComponent;
class UCameraComponent;

UCLASS()
class PARKOURAK_API AMainCharacterBB : public ACharacter
{
	GENERATED_BODY()

public:
	AMainCharacterBB(const FObjectInitializer& ObjectInitializer);
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;

	// Service Functions
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

	FCollisionQueryParams GetIgnoreCharacterParams() const;
	
	// Movement Inputs
	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void StartRunning();

	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void StopRunning();

	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void StartCrouch();
	
	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void StopCrouch();

	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void OnStartSlide();

	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void OnEndSlide();

	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void OnStartWallRun();

	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void OnEndWallRun();
	
	// Called to set the flag indicating the player jumped since the last update.
	UFUNCTION(BlueprintCallable, Category = "Player|Movement")
	void SetHasJumped();
	

	bool IsRunning() const;

	void PerformMantle(FVector StartPos,FVector EndPos, bool bTallMantle);

	bool IsInMantle() const;

	
	bool bPressedMainCharjump;

	
	UMainCharacterMovementComponent* GetMainCharacterMovementComponent();

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Player Camera")
	float StandardFOV = 90.f;
	
	//Crouch 
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement|Crouch")
	FVector CrouchEyeOffset;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement|Crouch")
	float CrouchSpeed;

	//Slide
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement|Slide")
	float SlideCameraTiltOffsetTarget = 2.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement|Slide")
	float SlideCameraTiltSpeed;

	//WallRun
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement|WallRun")
	float WallRunCameraTiltOffsetTarget = 10.f;

	//Mantle
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement|Mantle")
	float MantleSpeed = 20.f;

	//Run
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement|Running")
	float FovRunningChangeSpeed = 15.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement|Running")
	float FovRunningOffsetTarget = 5.f;

	
	virtual void Jump() override;
	virtual void StopJumping() override;


protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	//jump override
	virtual bool CanJumpInternal_Implementation() const override;


	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement) UMainCharacterMovementComponent* MainCharacterMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category=Camera) UCameraComponent* PlayerCamera;

private:
	
	//Run
	bool bHasRan = false;
	bool bIsRunning = false;
	float FovRunningOffset;

	// did the character jump since the last update?
	bool bHasJumped = false;
	
	//Sliding
	bool bIsSliding = false;
	float SlideCameraTiltOffset;

	//WallRun
	bool bIsInWallRun = false;
	float WallRunCameraTiltOffset;

	//Mantle
	float MantleAlpha;
	FVector StartPosition;
	FVector EndPosition;
	bool bIsInMantle = false;
	bool bIsTallMantle = false;
	
};

