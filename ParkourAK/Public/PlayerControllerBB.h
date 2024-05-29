// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputAction.h"
#include "InputActionValue.h"

#include "MainCharacterBB.h"

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerBB.generated.h"



class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class PARKOURAK_API APlayerControllerBB : public APlayerController
{
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Input|Character Movement")
	TObjectPtr<UInputAction> ActionLook = nullptr;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Input|Character Movement")
	TObjectPtr<UInputAction> ActionMove = nullptr;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Input|Character Movement")
	TObjectPtr<UInputAction> ActionJump = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Input|Character Movement")
	TObjectPtr<UInputAction> ActionToggleCrouch = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Input|Character Movement")
	TObjectPtr<UInputAction> ActionToggleSprint = nullptr;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Input|Character Movement")
	TObjectPtr<UInputMappingContext> InputMappingContent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Input|Character Animation")
	TObjectPtr<UInputAction> ActionPlayAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly) UAnimMontage* TestAnimation;


protected:
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;

	void HandleLook(const FInputActionValue& InputActionValue);
	void HandleMove(const FInputActionValue& InputActionValue);
	void HandleJump();
	void HandleCrouch();
	void StartSprint();
	void StopSprint();

	void PlayAnimation();


private:
	//Used to store a reference to the InputComponent cast to an EnhancedInputComponent
	UPROPERTY()
	TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = nullptr;

	//used to store a reference to the pawn we are controlling
	UPROPERTY()
	TObjectPtr<AMainCharacterBB> PlayerCharacter = nullptr;

	//used to store a reference to the PlayerCameraManager
	//UPROPERTY()
	//TObjectPtr<AMainCharacterPlayerCameraManagerBB> AMainCharacterPlayerCameraManagerClass = nullptr;
	GENERATED_BODY()

};






