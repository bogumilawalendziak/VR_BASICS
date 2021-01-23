// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "HeadMountedDisplay.h"
#include "cPlayerHandAnimBP.h"
#include "SteamVRChaperoneComponent.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // Motion controlelr for VR 

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRPlayer.generated.h"



UCLASS()
class MYPROJECT_API AVRPlayer : public ACharacter
{
	//GENERATED_BODY()

		/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UMotionControllerComponent* L_MotionController;
public:
	// Sets default values for this character's properties
	AVRPlayer();

	UFUNCTION(BlueprintNativeEvent, Category = "Input")
		void GripLeftHand_Pressed();
	UFUNCTION(BlueprintNativeEvent, Category = "Input")
		void GripRightHand_Pressed();
	UFUNCTION(BlueprintNativeEvent, Category = "Input")
		void GripLeftHand_Released();
	UFUNCTION(BlueprintNativeEvent, Category = "Input")
		void GripRightHand_Released();

	UPROPERTY(EditDefaultsOnly, Category = "Components")
		UPhysicsHandleComponent* PhysicsHandle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void CreateComponents();

	void CreateHandController(USceneComponent* a_compParent, FName a_strDisplayName, FName a_nameHandType);

	USkeletalMeshComponent * CreateHandMesh(UMotionControllerComponent * a_compParent, FName a_strDisplayName, FName a_nameHandType);

	void SetHandAnimationBlueprint(USkeletalMeshComponent* a_refHand);

	void MoveRight(float Axis);

	void MoveForward(float Axis);

	void CacheHandAnimInstances();
	
	UcPlayerHandAnimBP* m_refLeftHandAnimBP;
	UcPlayerHandAnimBP* m_refRightHandAnimBP;

	USkeletalMeshComponent* m_meshLeftHand;
	USkeletalMeshComponent* m_meshRightHand;

	float Reach = 100.0f;

	void Grab();

	
	void Release();

	const FHitResult GetFirstPhysicsBodyInReach();
	//UPhysicsHandleComponent* MyPhysicsHandle = nullptr;
};
