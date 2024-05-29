// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterPlayerCameraManagerBB.h"
#include "MainCharacterBB.h"
#include "MainCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

AMainCharacterPlayerCameraManagerBB::AMainCharacterPlayerCameraManagerBB()
{
	
}


/*
void AMainCharacterPlayerCameraManagerBB::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if(AMainCharacterBB* MainCharacter = Cast<AMainCharacterBB>(GetOwningPlayerController()->GetPawn()))
	{
		UMainCharacterMovementComponent* MainCharacterMovementComponent = MainCharacter->GetMainCharacterMovementComponent();

		//We need to get the default character capsule height, that's why we convert MainCharacter (it can be altered if we are crouched or not) 
		FVector TargetCrouchOffset = FVector(
			0,
			0,
			 MainCharacterMovementComponent->GetCrouchedHalfHeight() - MainCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
		);
		UE_LOG(LogTemp, Warning, TEXT("The float value is: %f"), TargetCrouchOffset.Z);
		//lerp = parto da 0, target 100, alpha = 0.7 -> risultato = 70
		FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));

		if (MainCharacterMovementComponent->IsCrouching())
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
			Offset -= TargetCrouchOffset;
		}
		else
		{
			CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
		}

		OutVT.POV.Location += Offset;

	}
}

*/