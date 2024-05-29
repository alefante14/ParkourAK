// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"

UMainCharacterMovementComponent::UMainCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
	//Sprint_MaxWalkSpeed = 800.0f;
	//Walk_MaxWalkSpeed = 100.0f;
}
#pragma region input
void UMainCharacterMovementComponent::SprintPressed()
{
	bWantsToSprint = true;
}

void UMainCharacterMovementComponent::SprintReleased()
{
	bWantsToSprint = false;
}

bool UMainCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}


#pragma endregion

void UMainCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	MainCharacterOwner = Cast<AMainCharacterBB>(GetOwner());
}

void UMainCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);    //commentabile?
	if (MovementMode == MOVE_Walking)
	{
		if (bWantsToSprint)
		{
			MaxWalkSpeed = Sprint_MaxWalkSpeed;
		}
		else
		{
			MaxWalkSpeed = Walk_MaxWalkSpeed;
		}
	}

	//Update Crouch Position based on the boolean
	//bWantsToCrouch

}

float UMainCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if(MovementMode != MOVE_Custom)return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return BrakingDecelerationSliding;
	case CMOVE_Wallrun:
		return 0.f;
		default:
			UE_LOG(LogTemp, Fatal, TEXT("Invalid movement Mode"))
			return -1.f;
	}
}

bool UMainCharacterMovementComponent::IsMovingOnGround() const
{
	//we are telling that the slide is a movement mode that's on the ground
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UMainCharacterMovementComponent::CanCrouchInCurrentState() const
{
	//We are telling not to crouch in air, this is Optional (DO NOT CROUCH IN AIR, ENABLE CROUCH WHEN LANDING)
	return Super::CanCrouchInCurrentState(); //&& IsMovingOnGround();
}

bool UMainCharacterMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump() || IsWallRunning();
}

bool UMainCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	bool bWasWallRunning = IsWallRunning();
	if(Super::DoJump(bReplayingMoves))
	{
		if (bWasWallRunning)
		{
			FVector Start = UpdatedComponent->GetComponentLocation();
			FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
			FVector End = bWallRunIsLeftWall ? Start - CastDelta : Start + CastDelta;
			auto Params = MainCharacterOwner->GetIgnoreCharacterParams();
			FHitResult WallHit;
			GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
			Velocity += WallHit.Normal * WallJumpOffForce;
			OnEndWallRun();
			UE_LOG(LogTemp,Warning,TEXT("Jumping from wall"))
		}
		return true;
	}
	return false;
}

float UMainCharacterMovementComponent::GetMaxSpeed() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return SlideMaxSpeed;
	case CMOVE_Wallrun:
		return WallRunMaxSpeed;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		return -1.f;
	}
}


//edge detection and collision check before sliding
void UMainCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	//Slide
	if(MovementMode == MOVE_Walking)
	{
		LastWallRun = LAST_None;
		if(bWantsToCrouch)
		{
			FHitResult PotentialSlideSurface;
			if (Velocity.SizeSquared() > pow(Slide_EnterMinSpeed,2) && GetSlideSurface((PotentialSlideSurface)))
			{
				EnterSlide();
			}
		}
	}

	
	if(IsCustomMovementMode(CMOVE_Slide))
	{
		if(!bWantsToCrouch) ExitSlide();
	}
	//TEMPORARY
	if(MainCharacterOwner->bPressedJump)
	{
		MainCharacterOwner->OnEndSlide();
	}

	// Try Mantle
	if(MainCharacterOwner->bPressedMainCharjump && !bIsInMantle)
	{
		if(TryMantle())
		{
			MainCharacterOwner->StopJumping();
		}
		else
		{
			{
				//Failed Mantle, reverting to jump
				MainCharacterOwner->bPressedMainCharjump = false;
				MainCharacterOwner->bPressedJump = true;
				MainCharacterOwner->CheckJumpInput(DeltaSeconds);
			}
		}
	}

	//Wallrun
	if(IsFalling())
	{
		TryWallRun();
	}
	
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UMainCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	//we check root motion here because after this update the cmc apply root motion to Rotation and consume our custom root motion
	if(!HasAnimRootMotion() && MovementMode == MOVE_Flying)
	{
		SetMovementMode(MOVE_Walking);
	}
	
	if(GetRootMotionSourceByID(MantleRMS_ID) && GetRootMotionSourceByID(MantleRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(MantleRMS_ID);
		bMantleTransitionFinished = true;
		UE_LOG(LogTemp,Warning,TEXT("Mantle Transition Root Motion Removed"))
	}
	/*
	if(GetRootMotionSourceByID(MantleRMS_ID) && GetRootMotionSourceByID(MantleRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(MantleRMS_ID);
		UE_LOG(LogTemp,Warning,TEXT("Vertical Root Motion Removed"))
	}
	if(GetRootMotionSourceByID(MantleForward_ID) && GetRootMotionSourceByID(MantleForward_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(MantleForward_ID);
		bMantleFinished = true;
		UE_LOG(LogTemp,Warning,TEXT("Horizontal Root Motion Removed "))
	}
	*/

	
}


//This is how you implement a Custom Physics 
void UMainCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime,Iterations);
		break;
	case CMOVE_Wallrun:
		PhysWallrun(deltaTime,Iterations);
		break;
		default:
			//this has never have to happen
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}

//Service
float UMainCharacterMovementComponent::CapHH() const
{
	return MainCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

float UMainCharacterMovementComponent::CapR() const
{
	return MainCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

#pragma region Slide

void UMainCharacterMovementComponent::EnterSlide()
{
	bWantsToCrouch = true;
	Velocity += Velocity.GetSafeNormal() * Slide_EnterImpulse;  //Velocity without Z (GetSafeNormal2D)
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
	FindFloor(UpdatedComponent->GetRelativeLocation(),CurrentFloor,true,nullptr);
	MainCharacterOwner->OnStartSlide();
}

void UMainCharacterMovementComponent::ExitSlide()
{
	bWantsToCrouch = false;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector,NewRotation,true,Hit);
	SetMovementMode(MOVE_Walking);
	MainCharacterOwner->OnEndSlide();
}



void UMainCharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	//Engine wants this
	if(deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	//RootMotion is the movement of a character's root bone in an animation (optional?)
	RestorePreAdditiveRootMotionVelocity();

	//exit slide conditions
	FHitResult SurfaceHit;
	if(!GetSlideSurface(SurfaceHit) || Velocity.SizeSquared() < pow(Slide_MinSpeed,2))
	{
		ExitSlide();
		StartNewPhysics(deltaTime,Iterations);
		return;
	}
	//Surface gravity
	Velocity += Slide_GravityForce * FVector::DownVector * deltaTime;

	//Strafe
	//Acceleration is the input from WASD, GetRightVector is used to get only AD input
	if(FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5f)
	{
		Acceleration = Acceleration.ProjectOnTo((UpdatedComponent->GetRightVector()));  
	}
	else
	{
		Acceleration = FVector::ZeroVector;
	}

	//Calc Velocity
	if(!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(deltaTime, Slide_Friction, true, GetMaxBrakingDeceleration());
	}
	ApplyRootMotionToVelocity(deltaTime);

	//Perform Move
	++Iterations;
	bJustTeleported = false;
	
	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	//FQuat OldRotation = UpdatedComponent->GetComponentLocation().ToOrientationQuat();
	FHitResult Hit(1.f);
	FVector Adjusted = Velocity * deltaTime;
	FVector VelPlaneDir = FVector::VectorPlaneProject(Velocity,SurfaceHit.Normal).GetSafeNormal();
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlaneDir, SurfaceHit.Normal).ToQuat();  //rotation from X axis movement and Z axis surface normal
	//This actually move the capsule, sweep stops the capsule before hitting something, Hit is going to store if we actually hit something
	SafeMoveUpdatedComponent(Adjusted,NewRotation,true,Hit);

	//Handle the impact
	if(Hit.Time < 1.f)
	{
		HandleImpact(Hit,deltaTime,Adjusted);
		//Go parallel with the wall you hit with less velocity
		SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
	}
	
	FHitResult NewSurfaceHit;
	if(!GetSlideSurface(NewSurfaceHit) || Velocity.SizeSquared() < pow(Slide_MinSpeed, 2))
	{
		ExitSlide();
	}	

	// Update Outgoing Velocity & Acceleration
	if(!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
	}
}

bool UMainCharacterMovementComponent::GetSlideSurface(FHitResult& Hit) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	return GetWorld()->LineTraceSingleByProfile(Hit,Start,End,ProfileName, MainCharacterOwner->GetIgnoreCharacterParams());
}

#pragma endregion

#pragma region Mantle

bool UMainCharacterMovementComponent::TryMantle()
{
	UE_LOG(LogTemp,Warning, TEXT("TryMantle"))
	//if is walking not crouched and is not falling return
	if(!(MovementMode == MOVE_Walking && !IsCrouching()) && !(MovementMode == MOVE_Falling)) return false;
	
	// Helper Variables
	FVector BaseLoc = UpdatedComponent->GetComponentLocation() + FVector::DownVector * CapHH();
	FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	auto Params = MainCharacterOwner->GetIgnoreCharacterParams();
	float MaxHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2 + MantleReachHeight;
	float CosMMWSA = FMath::Cos(FMath::DegreesToRadians(MantleMinWallSteepnessAngle));
	float CosMMSA = FMath::Cos(FMath::DegreesToRadians(MantleMaxSurfaceAngle));
	float CosMMAA = FMath::Cos(FMath::DegreesToRadians(MantleMaxAlignmentAngle));

	//Check Front Face
	FHitResult FrontHit;
	float CHeckDistance = FMath::Clamp(Velocity | Fwd, CapR() + 30, MantleMaxDistance);  //Distance is based on Velocity
	FVector FrontStart = BaseLoc + FVector::UpVector * (MaxStepHeight - 1);
	//generate lines from MaxStepHeight to the top and look for a hit
	for(int i = 0; i < 6; ++i)
	{
		//DrawDebugLine(GetWorld(),FrontStart, FrontStart + Fwd * CHeckDistance, FColor::Red, false, 3.f);

		if(GetWorld()->LineTraceSingleByProfile(FrontHit,FrontStart,FrontStart + Fwd * CHeckDistance, "BlockAll", Params)) break;
		FrontStart += FVector::UpVector * (2.f * CapHH() - (MaxStepHeight - 1)) / 5;  
	}
	if (!FrontHit.IsValidBlockingHit()) return false;  //Did we hit anything?
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	//if the wall isn't steep enough or we are not properly aligned with the wall
	if (FMath::Abs(CosWallSteepnessAngle) > CosMMWSA || (Fwd | -FrontHit.Normal) < CosMMAA) return false;

	// Check Height
	TArray<FHitResult> HeightHits;
	FHitResult SurfaceHit;
	FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, FrontHit.Normal).GetSafeNormal(); 
	float WallCos = FVector::UpVector | FrontHit.Normal;
	float WallSin = FMath::Sqrt(1 - WallCos * WallCos);								//Non capisco il WallSin qua (penso sia inutile)
	FVector TraceStart = FrontHit.Location + Fwd + WallUp * (MaxHeight - (MaxStepHeight - 1)) / WallSin;  //lancio un raggio parallelo al muro fino a max altezza scalabile e poi torno indietro per vedere se interseziono una piattaforma
	FVector TraceStartNoWall = FrontHit.Location + Fwd + WallUp * (MaxHeight - (MaxStepHeight - 1));  

	//DrawDebugLine(GetWorld(),TraceStart, FrontHit.Location + Fwd, FColor::Blue, false, 5.f);

	if (!GetWorld()->LineTraceMultiByProfile(HeightHits, TraceStart, FrontHit.Location + Fwd, "BlockAll", Params)) return false;  //multitest, il raggio potrebbe incrociare piú superfici, dobbiamo capire quella valida per la scalata
	for (const FHitResult& Hit : HeightHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			SurfaceHit = Hit;
			break;
		}
	}
	//if the surface we are mantling on is too steep, exit
	if (!SurfaceHit.IsValidBlockingHit() || (SurfaceHit.Normal | FVector::UpVector) < CosMMSA) return false;
	float Height = (SurfaceHit.Location - BaseLoc) | FVector::UpVector;

	if(Height > MaxHeight) return false;
	

	// Check Clearance
	float SurfaceCos = FVector::UpVector | SurfaceHit.Normal;
	float SurfaceSin = FMath::Sqrt(1 - SurfaceCos * SurfaceCos);
	FVector ClearCapLoc = SurfaceHit.Location + Fwd * CapR() + FVector::UpVector * (CapHH() + 1 + CapR() * 2 * SurfaceSin); //La capsula ha come origine il centro, va spostata su (l'ultima parte calcola l'altezza della superficie inclinata)
	FCollisionShape CapShape = FCollisionShape::MakeCapsule(CapR(), CapHH());
	if (GetWorld()->OverlapAnyTestByProfile(ClearCapLoc, FQuat::Identity, "BlockAll", CapShape, Params))
	{
				return false;
	}
	DrawDebugCapsule(GetWorld(), ClearCapLoc, CapHH(), CapR(), FQuat::Identity, FColor::Green, false, 5.f);

	UE_LOG(LogTemp,Warning, TEXT("Can Mantle"))

	//MANTLE START MANTLE START MANTLE START
	bIsInMantle = true;
	FVector ShortMantleTarget = GetMantleStartLocation(FrontHit, SurfaceHit, false);
	FVector TallMantleTarget = GetMantleStartLocation(FrontHit, SurfaceHit, true);
	
	bool bTallMantle = false;
	if (MovementMode == MOVE_Walking && Height > CapHH() * 2 + 10)
		bTallMantle = true;
	else if (MovementMode == MOVE_Falling && (Velocity | FVector::UpVector) < 0)
	{
		if (!GetWorld()->OverlapAnyTestByProfile(TallMantleTarget, FQuat::Identity, "BlockAll", CapShape, Params))
			bTallMantle = true;
	}
	FVector TransitionTarget = bTallMantle ? TallMantleTarget : ShortMantleTarget;

	DrawDebugCapsule(GetWorld(), TransitionTarget, CapHH(), CapR(), FQuat::Identity, FColor::Yellow, false, 5.f);

	//Introduce Mantle???
	MainCharacterOwner->PerformMantle(UpdatedComponent->GetComponentLocation(), ClearCapLoc, bTallMantle);
	return true;
}


FVector UMainCharacterMovementComponent::GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit,
	bool bTallMantle) const
{
	//Dobbiamo posizionare la capsula nella posizione di partenza dell'animazione (a seconda che sia Short or Tall)
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	float DownDistance = bTallMantle ? CapHH() * 2.f : MaxStepHeight - 1;
	FVector EdgeTangent = FVector::CrossProduct(SurfaceHit.Normal, FrontHit.Normal).GetSafeNormal();

	FVector MantleStart = SurfaceHit.Location;
	MantleStart += FrontHit.Normal.GetSafeNormal2D() * (2.f + CapR());
	MantleStart += UpdatedComponent->GetForwardVector().GetSafeNormal2D().ProjectOnTo(EdgeTangent) * CapR() * .3f; //to mantle a bit closer to character
	MantleStart += FVector::UpVector * CapHH();
	MantleStart += FVector::DownVector * DownDistance;
	MantleStart += FrontHit.Normal.GetSafeNormal2D() * CosWallSteepnessAngle * DownDistance; //bump out the capsule if the wall is not straight

	return MantleStart;
	
}

void UMainCharacterMovementComponent::SetMantleStatus(bool status)
{
	bIsInMantle = status;
}


#pragma endregion


#pragma region Wallrun

bool UMainCharacterMovementComponent::TryWallRun()
{

	if(!IsFalling()) return false;
	if (Velocity.SizeSquared2D() < pow(WallRunMinSpeed, 2)) return false;
	
	FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	auto Params = MainCharacterOwner->GetIgnoreCharacterParams();

	//controllare che vettore velocity sia allineato con la visuale del character
	if(!FMath::IsWithin(FMath::RadiansToDegrees(FMath::Acos(Velocity.GetSafeNormal2D() | Fwd)),0.f,85.f))
	{
		return false;
	}
	//are we high enough?
	FHitResult FloorHit;
	FVector BaseLoc = UpdatedComponent->GetComponentLocation() + FVector::DownVector * CapHH();
	if(	GetWorld()->LineTraceSingleByProfile(	FloorHit,BaseLoc,BaseLoc + FVector::DownVector * WallRunMinFloorDistance,"BlockAll", Params))
	{
		return false;
	}

	// Non dobbiamo superare WallrunMaxFallingSpeed con la velocita di caduta
	//UE_LOG(LogTemp,Warning,TEXT("vertical speed component : %f"), Velocity | FVector::DownVector)
	if((Velocity | FVector::DownVector) > WallRunMaxFallingSpeed)
	{
		return false;
	}
	

	//check if there's any wall right or left
	FHitResult LeftSideHit;
	FHitResult RightSideHit;
	
	FVector LeftHalfQuarter = - UpdatedComponent->GetRightVector().GetSafeNormal2D().RotateAngleAxis(45, UpdatedComponent->GetRightVector().ZAxisVector);
	FVector RightHalfQuarter = UpdatedComponent->GetRightVector().GetSafeNormal2D().RotateAngleAxis(-45, UpdatedComponent->GetRightVector().ZAxisVector);
	float WallDistanceLeft = FMath::Clamp(Velocity | LeftHalfQuarter * 0.5f,0, WallRunMaxWallDistance);
	float WallDistanceRight = FMath::Clamp(Velocity | RightHalfQuarter * 0.5f,0, WallRunMaxWallDistance);
	
	FVector Start = UpdatedComponent->GetComponentLocation();
	//FVector LeftEnd = Start  - (UpdatedComponent->GetRightVector().GetSafeNormal2D() * WallDistanceLeft);
	//FVector RightEnd = Start + (UpdatedComponent->GetRightVector().GetSafeNormal2D()) * WallDistanceRight ;
	FVector LeftEnd = Start  - (UpdatedComponent->GetRightVector().GetSafeNormal2D() * CapR() * 2);
	FVector RightEnd = Start  + (UpdatedComponent->GetRightVector().GetSafeNormal2D() * CapR() * 2);
	
	//DrawDebugLine(GetWorld(), Start, Start -  UpdatedComponent->GetRightVector().GetSafeNormal2D() * WallDistanceLeft, FColor::Cyan, false, 4.f);
	DrawDebugLine(GetWorld(), Start, LeftEnd, FColor::Cyan, false, 4.f);
	DrawDebugLine(GetWorld(), Start, RightEnd, FColor::Orange, false, 4.f);
	
	GetWorld()->LineTraceSingleByProfile(LeftSideHit,Start,LeftEnd,"BlockAll", Params);
	if(LeftSideHit.IsValidBlockingHit())
	{
		bWallRunIsLeftWall = true;
	}
	else
	{
		GetWorld()->LineTraceSingleByProfile(RightSideHit,Start,RightEnd,"BlockAll", Params);
		if(RightSideHit.IsValidBlockingHit())
		{
			bWallRunIsLeftWall = false;
		}
		else
		{
			return false;
		}
	}
	if(bWallRunIsLeftWall)
	{
		if(LastWallRun == LAST_Left) return false;
		LastWallRun = LAST_Left;
	}
	else
	{
		if(LastWallRun == LAST_Right) return false;
		LastWallRun = LAST_Right;
	}
	
	FVector ProjectedVelocity = FVector::VectorPlaneProject(Velocity,bWallRunIsLeftWall ? LeftSideHit.Normal : RightSideHit.Normal);
	if(ProjectedVelocity.SizeSquared2D() < pow(WallRunMinSpeed,2)) return false;
	UE_LOG(LogTemp,Warning,TEXT("Wallrunning!"))
	
	Velocity = ProjectedVelocity;
	Velocity.Z = FMath::Clamp(Velocity.Z, 0.f, WallRunMaxVerticalSpeed); 
	SetMovementMode(MOVE_Custom, CMOVE_Wallrun);
	MainCharacterOwner->OnStartWallRun();

	return true;
	
	
	
}

void UMainCharacterMovementComponent::OnEndWallRun()
{
	MainCharacterOwner->OnEndWallRun();
	SetMovementMode(MOVE_Falling);
	UE_LOG(LogTemp,Warning,TEXT("Ending WallRun"))
}

void UMainCharacterMovementComponent::PhysWallrun(float deltaTime, int32 Iterations)
{
	//Engine wants this
	if(deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	if (!CharacterOwner)
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}
	
	bJustTeleported = false;
	float remainingTime = deltaTime;
	// Perform the move
	while ( (remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) )
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		
		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
		FVector End = bWallRunIsLeftWall ? Start - CastDelta : Start + CastDelta;
		auto Params = MainCharacterOwner->GetIgnoreCharacterParams();
		float SinPullAwayAngle = FMath::Sin(FMath::DegreesToRadians(WallRunPullAwayAngle));
		FHitResult WallHit;
		GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
		bool bWantsToPullAway = WallHit.IsValidBlockingHit() && !Acceleration.IsNearlyZero() && (Acceleration.GetSafeNormal() | WallHit.Normal) > SinPullAwayAngle;
		if (!WallHit.IsValidBlockingHit() || bWantsToPullAway)
		{
			OnEndWallRun();
			StartNewPhysics(remainingTime, Iterations);
			return;
		}
		// Clamp Acceleration
		Acceleration = FVector::VectorPlaneProject(Acceleration, WallHit.Normal);
		Acceleration.Z = 0.f;
		// Apply acceleration
		CalcVelocity(timeTick, 0.f, false, GetMaxBrakingDeceleration());
		Velocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
		float TangentAccel = Acceleration.GetSafeNormal() | Velocity.GetSafeNormal2D();
		bool bVelUp = Velocity.Z > 0.f;
		Velocity.Z += GetGravityZ() * WallRunGravityScaleCurve->GetFloatValue(bVelUp ? 0.f : TangentAccel) * timeTick;
		if (Velocity.SizeSquared2D() < pow(WallRunMinSpeed, 2) || Velocity.Z < -WallRunMaxVerticalSpeed)
		{
			OnEndWallRun();
			StartNewPhysics(remainingTime, Iterations);
			return;
		}
		
		// Compute move parameters
		const FVector Delta = timeTick * Velocity; // dx = v * dt
		const bool bZeroDelta = Delta.IsNearlyZero();
		if ( bZeroDelta )
		{
			remainingTime = 0.f;
		}
		else
		{
			FHitResult Hit;
			SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
			FVector WallAttractionDelta = -WallHit.Normal * WallAttractionForce * timeTick;
			SafeMoveUpdatedComponent(WallAttractionDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
		}
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick; // v = dx / dt
	}

	
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
	FVector End = bWallRunIsLeftWall ? Start - CastDelta : Start + CastDelta;
	auto Params = MainCharacterOwner->GetIgnoreCharacterParams();
	FHitResult FloorHit, WallHit;
	GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
	GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + WallRunMinFloorDistance * .5f), "BlockAll", Params);
	if (FloorHit.IsValidBlockingHit() || !WallHit.IsValidBlockingHit() || Velocity.SizeSquared2D() < pow(WallRunMinSpeed, 2))
	{
		OnEndWallRun();
	}

	
}

#pragma endregion 