// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBB.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MainCharacterMovementComponent.h"
#include "MainCharacterPlayerCameraManagerBB.h"


class UMainCharacterMovementComponent;

void APlayerControllerBB::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	//store a reference to the player's Pawn
	PlayerCharacter = Cast<AMainCharacterBB>(aPawn);
	checkf(PlayerCharacter, TEXT("APlayerCOntrollerBB derived classes should only posess ACharacterBB derived pawns"));

	// Get a reference to the EnhancedInputComponent
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);  //InputComponent is an actor member
	checkf(EnhancedInputComponent, TEXT("Unable to get reference to the EnhancedInputComponent."));

	//Get a reference of the PlayerCameraManager
	//AMainCharacterPlayerCameraManagerClass = AMainCharacterPlayerCameraManagerBB::StaticClass();
	
	// Get the local player subsystem
		// Just a local variable, we dont need to refer to it again after this
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	checkf(InputSubsystem,
		TEXT("Unable to get reference to the EnhancedInputLocalPlayerSubsystem."));

	// Wipe existing mappings, and add our mapping.
	checkf(InputMappingContent, TEXT("InputMappingContent was not specified."));
	InputSubsystem->ClearAllMappings();
	InputSubsystem->AddMappingContext(InputMappingContent, 0);



	// Bind the input actions
	// Only attempt to bind if a valid value was provided
	if (ActionMove) {
		EnhancedInputComponent->BindAction(ActionMove,ETriggerEvent::Triggered, this, &APlayerControllerBB::HandleMove);  //sto implementando una funzione virtuale
	}
	if (ActionLook) {
		EnhancedInputComponent->BindAction(ActionLook, ETriggerEvent::Triggered, this, &APlayerControllerBB::HandleLook);
	}
	if (ActionJump) {
		EnhancedInputComponent->BindAction(ActionJump, ETriggerEvent::Triggered, this, &APlayerControllerBB::HandleJump);
	}
	if (ActionToggleCrouch) {
		EnhancedInputComponent->BindAction(ActionToggleCrouch, ETriggerEvent::Triggered, this, &APlayerControllerBB::HandleCrouch);
	}
	if (ActionToggleSprint) {
		EnhancedInputComponent->BindAction(ActionToggleSprint, ETriggerEvent::Started, this, &APlayerControllerBB::StartSprint);
		EnhancedInputComponent->BindAction(ActionToggleSprint, ETriggerEvent::Completed, this, &APlayerControllerBB::StopSprint);
	}
	if(ActionPlayAnimation)
	{
		EnhancedInputComponent->BindAction(ActionPlayAnimation, ETriggerEvent::Triggered, this, &APlayerControllerBB::PlayAnimation);
	}

}


void APlayerControllerBB::OnUnPossess()
{
	//Unbind things here
	EnhancedInputComponent->ClearActionBindings();

	Super::OnUnPossess();
}

void APlayerControllerBB::HandleLook(const FInputActionValue& InputActionValue)
{
	const FVector2D LookingVector = InputActionValue.Get<FVector2D>();

	if (PlayerCharacter) {
		AddYawInput(LookingVector.X);
		AddPitchInput(LookingVector.Y);
	}
}

void APlayerControllerBB::HandleMove(const FInputActionValue& InputActionValue)
{
	//Value is a 2D Vector
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	// Add movement to the Player's Character Pawn
	if (PlayerCharacter)
	{
		PlayerCharacter->AddMovementInput(PlayerCharacter->GetActorForwardVector(), MovementVector.Y);
		PlayerCharacter->AddMovementInput(PlayerCharacter->GetActorRightVector(), MovementVector.X);
	}
}

void APlayerControllerBB::HandleJump()
{
	if (PlayerCharacter)
	{
		PlayerCharacter->UnCrouch();
		PlayerCharacter->Jump();
	}
}


void APlayerControllerBB::HandleCrouch()
{
	if (PlayerCharacter && PlayerCharacter->bIsCrouched)
	{
		PlayerCharacter->StopCrouch();

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Stop running"));
		//PlayerCharacter->StopRunning();
		PlayerCharacter->StartCrouch();
	}
}

void APlayerControllerBB::StartSprint()
{
	if (!PlayerCharacter->IsRunning())
	{
		PlayerCharacter->UnCrouch();
	}
	PlayerCharacter->StartRunning();

}

void APlayerControllerBB::StopSprint()
{
	PlayerCharacter->StopRunning();

}

void APlayerControllerBB::PlayAnimation()
{
	PlayerCharacter->GetMainCharacterMovementComponent()->SetMovementMode(MOVE_Flying);
	float check = PlayerCharacter->PlayAnimMontage(TestAnimation,1);
	UE_LOG(LogTemp,Warning,TEXT("Keybind animation started : %f"), check)
}





