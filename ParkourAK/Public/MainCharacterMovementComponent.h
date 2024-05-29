// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MainCharacterBB.h"

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainCharacterMovementComponent.generated.h"

class AMainCharacterBB;

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None	UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_Wallrun UMETA(DisplayName = "Wallrun"),
	CMOVE_MAX	UMETA(Hidden)
};

UCLASS()
class PARKOURAK_API UMainCharacterMovementComponent : public UCharacterMovementComponent
{
	
	
public:
	UMainCharacterMovementComponent();

	//Transient (Property is transient, meaning it will not be saved or loaded. Properties tagged this way will be zero-filled at load time)
	UPROPERTY(Transient) AMainCharacterBB* MainCharacterOwner = nullptr;

	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();

	//UFUNCTION(BlueprintCallable) void CrouchPressed();

	UFUNCTION(BlueprintPure) bool IsWallRunning() const { return IsCustomMovementMode(CMOVE_Wallrun); }
	UFUNCTION(BlueprintPure) bool WallrunIsLeft() const { return bWallRunIsLeftWall; }
	
	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;

	virtual bool CanAttemptJump() const override;
	virtual bool DoJump(bool bReplayingMoves) override;
	virtual float GetMaxSpeed() const override;
	
	void SetMantleStatus(bool status);

	
protected:
	virtual void InitializeComponent() override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual float GetMaxBrakingDeceleration() const override;



	
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
private:
	//Slide
	void EnterSlide();
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit) const;

	//Mantle
	bool TryMantle();
	FVector GetMantleStartLocation(FHitResult FrontHit,FHitResult SurfaceHit, bool bTallMantle) const;

	
	//Wallrun
	bool TryWallRun();
	void PhysWallrun(float deltaTime, int32 Iterations);
	void OnEndWallRun();

	
	//Service
	float CapHH() const;
	float CapR() const;
private:
	//Parameters
	UPROPERTY(EditDefaultsOnly) float Sprint_MaxWalkSpeed = 750.0f;
	UPROPERTY(EditDefaultsOnly) float Walk_MaxWalkSpeed = 500.0f;
	//Sliding
	UPROPERTY(EditDefaultsOnly) float Slide_EnterMinSpeed = 600.0f;
	UPROPERTY(EditDefaultsOnly) float Slide_MinSpeed = 200.0f;
	UPROPERTY(EditDefaultsOnly) float Slide_EnterImpulse = 500.0f;
	UPROPERTY(EditDefaultsOnly) float Slide_GravityForce = 5000.0f;
	UPROPERTY(EditDefaultsOnly) float Slide_Friction = 1.3f;
	UPROPERTY(EditDefaultsOnly) float BrakingDecelerationSliding=1000.f;
	UPROPERTY(EditDefaultsOnly) float SlideMaxSpeed=1000.f;

	//Mantle
	UPROPERTY(EditDefaultsOnly) float MantleMaxDistance = 200;
	UPROPERTY(EditDefaultsOnly) float MantleReachHeight = 50;
	UPROPERTY(EditDefaultsOnly) float MinMantleDepth = 30;
	UPROPERTY(EditDefaultsOnly) float MantleMinWallSteepnessAngle = 75;
	UPROPERTY(EditDefaultsOnly) float MantleMaxSurfaceAngle = 40;
	UPROPERTY(EditDefaultsOnly) float MantleMaxAlignmentAngle = 45;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TallMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionTallMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ShortMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionShortMantleMontage;
	//Wallrun
	UPROPERTY(EditDefaultsOnly) float WallRunMaxWallDistance = 300;
	UPROPERTY(EditDefaultsOnly) float WallRunMaxFallingSpeed = 350;
	UPROPERTY(EditDefaultsOnly) float WallRunMinFloorDistance = 40;
	UPROPERTY(EditDefaultsOnly) float WallJumpOffForce = 600.f;
	UPROPERTY(EditDefaultsOnly) float WallRunMaxVerticalSpeed=200.f;
	UPROPERTY(EditDefaultsOnly) float WallRunPullAwayAngle=75;
	UPROPERTY(EditDefaultsOnly) UCurveFloat* WallRunGravityScaleCurve;
	UPROPERTY(EditDefaultsOnly) float WallRunMinSpeed=800.f;
	UPROPERTY(EditDefaultsOnly) float WallRunMaxSpeed=900.f;
	UPROPERTY(EditDefaultsOnly) float WallAttractionForce = 200.f;
	
	
	bool bWallRunIsLeftWall = false;
	enum LWR
	{
		LAST_None,
		LAST_Left,
		LAST_Right,
	};
	LWR LastWallRun = LAST_None;
	
	bool bWantsToSprint = false;
	bool bMantleTransitionFinished = false;
	bool bIsInMantle = false;

	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;
	UPROPERTY(Transient) UAnimMontage* TransitionQueuedMontage;
	float TransitionQueuedMontageSpeed;

	//TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA


	TSharedPtr<FRootMotionSource_MoveToForce> MantleRMS;
	TSharedPtr<FRootMotionSource_MoveToForce> MantleForwardRMS;
	int MantleRMS_ID;
	FVector MantleStartPosition;
	FVector MantleTargetPosition;
	bool bTallMantleGlobal;
	bool bMantleFinished;
	// TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA TEST AREA
	GENERATED_BODY()
	
};
