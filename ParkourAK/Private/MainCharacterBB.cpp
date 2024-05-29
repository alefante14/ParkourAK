// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterBB.h"
#include "Camera/CameraComponent.h"
#include "MainCharacterMovementComponent.h"
#include "SNegativeActionButton.h"

// Sets default values
AMainCharacterBB::AMainCharacterBB(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	MainCharacterMovementComponent = Cast<UMainCharacterMovementComponent>(GetCharacterMovement());
	MainCharacterMovementComponent->SetIsReplicated(true);

	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera Component"));
	PlayerCamera->SetupAttachment(GetRootComponent());
	PlayerCamera->SetRelativeLocation(FVector(0.f,0.f,64.f));  //change it dynamically?
	PlayerCamera->bUsePawnControlRotation = true;
	
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//SetActorTickInterval(0.5f);
	SetActorTickEnabled(true);

	
	//Some Variable Initialization
	//JumpMaxCount = 2;
	//JumpMaxHoldTime = 0.7f;

	//Run
	FovRunningOffset = 0.f;
	
	//Crouch
	CrouchEyeOffset = FVector(0.f);
	CrouchSpeed = 12.f;

	//Slide
	SlideCameraTiltOffset = 0.f;
	SlideCameraTiltSpeed = 24.f;

	//WallRun
	WallRunCameraTiltOffset = 0.f;

	//Mantle
	MantleAlpha = 0.f;

	PlayerCamera->SetFieldOfView(StandardFOV);
}



// Called when the game starts or when spawned
void AMainCharacterBB::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMainCharacterBB::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	//standard linear interpolation
	float CrouchInterpTime = FMath::Min(1.f, CrouchSpeed * DeltaTime);
	CrouchEyeOffset =  (1.f - CrouchInterpTime) * CrouchEyeOffset;

	//not implemented
	float SlideInterpTime = FMath::Min(1.f, SlideCameraTiltSpeed * DeltaTime);
	SlideCameraTiltOffset = (1.f - SlideInterpTime) * SlideCameraTiltOffset;

	
	float WallRunInterpTime = FMath::Min(1.f, SlideCameraTiltSpeed * DeltaTime);
	WallRunCameraTiltOffset = (1.f - WallRunInterpTime) * WallRunCameraTiltOffset;

	float FovRunningInterpTime = FMath::Min(1.f, FovRunningChangeSpeed * DeltaTime);
	FovRunningOffset = (1.f - FovRunningInterpTime) * FovRunningOffset;
	


	if(bIsInMantle)
	{
		MantleAlpha += DeltaTime * (bIsTallMantle ? MantleSpeed / 2 : MantleSpeed);
		MantleAlpha = FMath::Clamp(MantleAlpha, 0.f,1.f);
		FVector CurrentPos = FMath::Lerp(StartPosition,EndPosition,	MantleAlpha);
		SetActorLocation(CurrentPos);

		if(MantleAlpha >= 1.f)
		{
			bIsInMantle = false;
			MainCharacterMovementComponent->SetMantleStatus(bIsInMantle);
			MantleAlpha = 0.f;
		}
	}
/*
#pragma region PRINT

	GEngine->AddOnScreenDebugMessage(-1, 0.49f, FColor::Silver,
		*(FString::Printf(
			TEXT("Movement - IsCrouched:%d | IsSprinting:%d"), bIsCrouched, bIsRunning)));
	GEngine->AddOnScreenDebugMessage(-1, 0.99f, FColor::Red,
	*(FString::Printf(
		TEXT("Velocity:%f"), MainCharacterMovementComponent->Velocity.SizeSquared2D())));
#pragma endregion
*/
}

// Called to bind functionality to input
void AMainCharacterBB::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//movement flags
void AMainCharacterBB::StartRunning()
{
	MainCharacterMovementComponent->SprintPressed();
	bIsRunning = true;
	FovRunningOffset = FovRunningOffsetTarget - FovRunningOffset;
	
}

void AMainCharacterBB::StopRunning()
{
	MainCharacterMovementComponent->SprintReleased();
	bIsRunning = false;
	if(!bIsSliding)
		FovRunningOffset = FovRunningOffsetTarget - FovRunningOffset;
}

void AMainCharacterBB::StartCrouch()
{
	//MainCharacterMovementComponent->CrouchPressed();
	Crouch();
}

void AMainCharacterBB::StopCrouch()
{
	//MainCharacterMovementComponent->CrouchPressed();
	UnCrouch();
}

//Service Crouch Functions
/*
* Called when Character crouches. Called on non-owned Characters through bIsCrouched replication.
* @param	HalfHeightAdjust		difference between default collision half-height, and actual crouched capsule half-height.
* @param	ScaledHalfHeightAdjust	difference after component scale is taken in to account.
* BaseEyeHeight = 64.f 	Base eye height above collision center. Is the middle of the height of the character
*
* constexpr float EyeHeightRatio = 0.8f;	// how high the character's eyes are, relative to the crouched height
* CrouchedEyeHeight = CharacterMovement->GetCrouchedHalfHeight() * EyeHeightRatio;
*/
void AMainCharacterBB::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if(HalfHeightAdjust == 0.f)
	{
		return;
	}
	//Mi salvo l'altezza degli occhi da in piedi
	float StartBaseEyeHeight = BaseEyeHeight;
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	//Parte da 0
	//altezza occhi in piedi - altezza occhi accovacciato + differenza tra capsula in piedi e accovacciata
	CrouchEyeOffset.Z += StartBaseEyeHeight - BaseEyeHeight + HalfHeightAdjust;
	PlayerCamera->SetRelativeLocation(FVector(0.f,0.f,BaseEyeHeight), false);

}

void AMainCharacterBB::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if(HalfHeightAdjust == 0.f)
	{
		return;
	}
	float StartBaseEyeHeight = BaseEyeHeight;
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	CrouchEyeOffset.Z += StartBaseEyeHeight - BaseEyeHeight - HalfHeightAdjust;
	PlayerCamera->SetRelativeLocation(FVector(0.f,0.f,BaseEyeHeight), false);

}

void AMainCharacterBB::OnStartSlide()
{
	//PlayerCamera->SetRelativeRotation(FRotator(0.f,0.f,SlideCameraTiltOffsetTarget));
	//SlideCameraTiltOffset = SlideCameraTiltOffsetTarget;
	bIsSliding = true;
	UE_LOG(LogTemp, Warning, TEXT("StartSlide"));
}

void AMainCharacterBB::OnEndSlide()
{
	//layerCamera->SetRelativeRotation(FRotator(0.f,0.f,0.f));
	//SlideCameraTiltOffset = SlideCameraTiltOffsetTarget - SlideCameraTiltOffset;
	UE_LOG(LogTemp, Warning, TEXT("EndSlide"));
	bIsSliding = false;
}

void AMainCharacterBB::OnStartWallRun()
{
	bIsInWallRun = true;
	if(MainCharacterMovementComponent->WallrunIsLeft())
	{
		WallRunCameraTiltOffset = WallRunCameraTiltOffsetTarget;
	}
	else
	{
		WallRunCameraTiltOffset = -WallRunCameraTiltOffsetTarget;
	}
}

void AMainCharacterBB::OnEndWallRun()
{
	bIsInWallRun = false;
	if(MainCharacterMovementComponent->WallrunIsLeft())
	{
		WallRunCameraTiltOffset = WallRunCameraTiltOffsetTarget - WallRunCameraTiltOffset;
	}
	else
	{
		WallRunCameraTiltOffset = -(WallRunCameraTiltOffsetTarget + WallRunCameraTiltOffset);
	}
}


void AMainCharacterBB::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if(PlayerCamera)
	{
		PlayerCamera->GetCameraView(DeltaTime, OutResult);
		if(MainCharacterMovementComponent->IsMovingOnGround())
		{
			//recalculate camera when start/end crouch
			OutResult.Location += CrouchEyeOffset;
		}
		if(bIsRunning || MainCharacterMovementComponent->IsCustomMovementMode(CMOVE_Slide))
		{
			OutResult.FOV += FovRunningOffsetTarget - FovRunningOffset;
		}
		else
		{
			OutResult.FOV += FovRunningOffset;
		}
		//slide tilt
		if(MainCharacterMovementComponent->IsCustomMovementMode(CMOVE_Slide))
		{
			//OutResult.Rotation += FRotator(0.f,0.f,SlideCameraTiltOffsetTarget - SlideCameraTiltOffset);
		}
		else
		{
			//OutResult.Rotation += FRotator(0.f,0.f,SlideCameraTiltOffset);
		}
		//WallRun tilt
		if(MainCharacterMovementComponent->IsCustomMovementMode(CMOVE_Wallrun))
		{
			if(MainCharacterMovementComponent->WallrunIsLeft())
			{
				OutResult.Rotation += FRotator(0.f,0.f,WallRunCameraTiltOffsetTarget - WallRunCameraTiltOffset);
			}
			else
			{
				OutResult.Rotation += FRotator(0.f,0.f,-(WallRunCameraTiltOffsetTarget + WallRunCameraTiltOffset));
			}
		}
		else
		{
			OutResult.Rotation += FRotator(0.f,0.f,WallRunCameraTiltOffset);
		}

		
	}
}

//setting up jump conditions
bool AMainCharacterBB::CanJumpInternal_Implementation() const
{
	return JumpIsAllowedInternal();
}

void AMainCharacterBB::Jump()
{
	bPressedMainCharjump = true;

	Super::Jump();

	bPressedJump = false;
}

void AMainCharacterBB::StopJumping()
{
	Super::StopJumping();
	bPressedMainCharjump = false;
}

FCollisionQueryParams AMainCharacterBB::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}


void AMainCharacterBB::SetHasJumped()
{
	bHasJumped = true;
}

bool AMainCharacterBB::IsRunning() const
{
	return bIsRunning;
}

//Movement Update
void AMainCharacterBB::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	Super::AddMovementInput(WorldDirection, ScaleValue, bForce);
}


UMainCharacterMovementComponent* AMainCharacterBB::GetMainCharacterMovementComponent()
{
	return MainCharacterMovementComponent;
}



void AMainCharacterBB::PerformMantle(FVector StartPos,FVector EndPos, bool bTallMantle)
{
	StartPosition = StartPos;
	EndPosition = EndPos;
	bIsInMantle = true;
	bIsTallMantle = bTallMantle;
}

bool AMainCharacterBB::IsInMantle() const
{
	return bIsInMantle;
}



