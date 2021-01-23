// Fill out your copyright notice in the Description page of Project Settings.
#include "VRPlayer.h"
#include "cPlayerHandAnimBP.h"
#include "HeadMountedDisplay.h"
#include "SteamVRChaperoneComponent.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // Motion controlelr for VR 
#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Pawn.h"
#include "XRMotionControllerBase.h" 
#include "Engine.h"


// Sets default values
AVRPlayer::AVRPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//FindPhysicsHandleComponent();
	CreateComponents();
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
	GetCharacterMovement()->bOrientRotationToMovement = true; // orientacja w kierunku ruchu 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); //rate of roation


	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AVRPlayer::BeginPlay()
{
	Super::BeginPlay();
	CacheHandAnimInstances();
	
}

// Called every frame
void AVRPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation, OUT PlayerViewPointRotation);
	
	FVector LineTraceEnd = PlayerViewPointLocation + PlayerViewPointRotation.Vector() * 10.0f;
	if (PhysicsHandle->GrabbedComponent	) {
		PhysicsHandle->SetTargetLocation(LineTraceEnd);
	
	}
	

}

// Called to bind functionality to input
void AVRPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("GripLeftHand", IE_Pressed, this, &AVRPlayer::GripLeftHand_Pressed);
	PlayerInputComponent->BindAction("GripRightHand", IE_Pressed, this, &AVRPlayer::GripRightHand_Pressed);
	PlayerInputComponent->BindAction("GripLeftHand", IE_Released, this, &AVRPlayer::GripLeftHand_Released);
	PlayerInputComponent->BindAction("GripRightHand", IE_Released, this, &AVRPlayer::GripRightHand_Released);
	PlayerInputComponent->BindAxis("MoveForward", this, &AVRPlayer::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AVRPlayer::MoveRight);
}

void AVRPlayer::CreateComponents()
{


	GetCapsuleComponent()->InitCapsuleSize(150.f, 96.0f);

	//Chaperone component that is required for Steam VR plugin
	USteamVRChaperoneComponent* chaperone = CreateDefaultSubobject<USteamVRChaperoneComponent>(TEXT("SteamVR Chaperone"));

	//Create a scene component that will act as the parent for the camera. This might come in handy later.
	USceneComponent* compVRCameraRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VR Camera Root"));
	compVRCameraRoot->SetupAttachment(GetCapsuleComponent());
	compVRCameraRoot->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	compVRCameraRoot->SetRelativeScale3D(FVector::OneVector);

	//Create a camera component and attach this to the camera root.
	UCameraComponent* compVRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("HMD Camera"));
	compVRCamera->SetupAttachment(compVRCameraRoot);
	compVRCamera->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	compVRCamera->SetRelativeScale3D(FVector::OneVector);

	CreateHandController(compVRCameraRoot, "MC_Left", FXRMotionControllerBase::LeftHandSourceId);
	CreateHandController(compVRCameraRoot, "MC_Right", FXRMotionControllerBase::RightHandSourceId);


	
}

void AVRPlayer::CreateHandController(USceneComponent* a_compParent, FName a_strDisplayName, FName a_nameHandType)
{
	UMotionControllerComponent* compMotionController = CreateDefaultSubobject<UMotionControllerComponent>(a_strDisplayName);
	compMotionController->SetRelativeLocationAndRotation(FVector::ZeroVector, FQuat::Identity);
	compMotionController->SetRelativeScale3D(FVector::OneVector);

	compMotionController->MotionSource = a_nameHandType;
	compMotionController->SetupAttachment(a_compParent);

	//Create the hand mesh for visualization
	
	FName strMeshDisplayName = a_nameHandType == FXRMotionControllerBase::LeftHandSourceId ? FName(TEXT("Hand_Left")) : FName(TEXT("Hand_Right"));
	USkeletalMeshComponent* refHandMesh = CreateHandMesh(compMotionController, strMeshDisplayName, a_nameHandType);
	if (a_nameHandType == FXRMotionControllerBase::LeftHandSourceId)
	{
		m_meshLeftHand = refHandMesh;
		UCapsuleComponent* CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
		CollisionComp->SetupAttachment(refHandMesh);
		CollisionComp->SetCapsuleHalfHeight(20.0f);
		CollisionComp->SetCapsuleRadius(20.0f);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else{
		m_meshRightHand = refHandMesh;
		UCapsuleComponent* CollisionComp2 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp2"));
		CollisionComp2->SetupAttachment(refHandMesh);
		CollisionComp2->SetCapsuleHalfHeight(20.0f);
		CollisionComp2->SetCapsuleRadius(20.0f);
		CollisionComp2->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

USkeletalMeshComponent* AVRPlayer::CreateHandMesh(UMotionControllerComponent* a_compParent, FName a_strDisplayName, FName a_nameHandType)
{
	USkeletalMeshComponent* refComponentHand = NULL;

	//Find the default cube that ships with the engine content
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> HandMeshObject(TEXT("SkeletalMesh'/Game/VirtualReality/Mannequin/Character/Mesh/MannequinHand_Right'"));
	//E: / Projekty / Unreal / MyProject / Content / Blueprints / BaseGame / VirtualReality / Mannequin / Character / Mesh / MannequinHand_Right.uasset
		//
	if (!HandMeshObject.Object)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not load the default cube for hand mesh"));
		return NULL;
	}

	//create the mesh component
	refComponentHand = CreateDefaultSubobject<USkeletalMeshComponent>(a_strDisplayName);
	//set the mesh to the component
	refComponentHand->SetSkeletalMesh(HandMeshObject.Object, true);

	//Set the defaults
	refComponentHand->SetAutoActivate(true);
	refComponentHand->SetVisibility(true);
	refComponentHand->SetHiddenInGame(false);
	FQuat qRotation = FQuat::Identity;
	FVector vec3Scale = FVector::OneVector;
	if (a_nameHandType == FXRMotionControllerBase::LeftHandSourceId)
	{
		qRotation = FQuat(FVector(1, 0, 0), FMath::DegreesToRadians(90));
		vec3Scale = FVector(1, -1, 1);
	
	}
	else
	{
		qRotation = FQuat(FVector(1, 0, 0), FMath::DegreesToRadians(270));
	}
	refComponentHand->SetRelativeLocationAndRotation(FVector::ZeroVector, qRotation);
	refComponentHand->SetRelativeScale3D(vec3Scale);
	refComponentHand->SetupAttachment(a_compParent);
	
	SetHandAnimationBlueprint(refComponentHand);
	
	return refComponentHand;
}

void AVRPlayer::SetHandAnimationBlueprint(USkeletalMeshComponent * a_refHand)
{
	static ConstructorHelpers::FObjectFinder<UAnimBlueprintGeneratedClass> HandAnimBP(TEXT("AnimBlueprintGeneratedClass'/Game/VirtualReality/Mannequin/Animations/RightHand_AnimBP.RightHand_AnimBP_C'"));
	if (HandAnimBP.Succeeded())
	{
		a_refHand->AnimClass = HandAnimBP.Object;
		a_refHand->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		a_refHand->SetAnimInstanceClass(HandAnimBP.Object);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not load the hand anim BP"));
	}

	
}
void AVRPlayer::GripLeftHand_Pressed_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Left Hand Grip Pressed"));
	m_refLeftHandAnimBP->SetGripValue(1.0f);
	Grab();
}

void AVRPlayer::GripRightHand_Pressed_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Right Hand Grip Pressed"));
	m_refRightHandAnimBP->SetGripValue(1.0f);
	Grab();
}

void AVRPlayer::GripLeftHand_Released_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Left Hand Grip Released"));
	m_refLeftHandAnimBP->SetGripValue(0.0f);
	Release();
}

void AVRPlayer::GripRightHand_Released_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Right Hand Grip Released"));
	m_refRightHandAnimBP->SetGripValue(0.0f);
	Release();
}

void AVRPlayer::MoveRight(float Axis)
{

	if (Axis != 0.0f)
	{
		if (Controller)
		{
			
			FRotator Rotation = GetBaseAimRotation();

			Rotation.Normalize();
			FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, Axis);

		}
	}

}
	
	

void AVRPlayer::MoveForward(float Axis)
{
	

	if (Axis != 0.0f)
	{
		if (Controller)
		{
		
		
			FRotator Rotation = GetBaseAimRotation();

			Rotation.Normalize();
			FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction,Axis);
		
	}
		
	}
}
void AVRPlayer::CacheHandAnimInstances()
{
	m_refLeftHandAnimBP = (UcPlayerHandAnimBP*)(m_meshLeftHand->GetAnimInstance());
	
	if (!IsValid(m_refLeftHandAnimBP))
		UE_LOG(LogTemp, Error, TEXT("Could not cast Hand Anim to the right class"));
	m_refRightHandAnimBP = (UcPlayerHandAnimBP*)(m_meshRightHand->GetAnimInstance());
	if (!IsValid(m_refRightHandAnimBP))
		UE_LOG(LogTemp, Error, TEXT("Could not cast Hand Anim to the right class"));
};


void AVRPlayer::Grab()
{
	UE_LOG(LogTemp, Error, TEXT("Grab pressed")); 
	//Line trace and see iof erach any actors with physic body collision chanel set 
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent();
	auto ActorHit = HitResult.GetActor();
	//If we hit cometing then attach a physics handle
	if (ActorHit != nullptr) {
		PhysicsHandle->GrabComponentAtLocationWithRotation(
	
			ComponentToGrab, NAME_None, ComponentToGrab->GetOwner()->GetActorLocation(),
				
			ComponentToGrab->GetComponentRotation()); // allow rotation

	}
	
};


void AVRPlayer::Release()
{
	UE_LOG(LogTemp, Error, TEXT("Grab released"));
};

const FHitResult AVRPlayer::GetFirstPhysicsBodyInReach() {
	//Get the player viewpoint this tick

	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation, OUT PlayerViewPointRotation);

	FVector LineTraceEnd = PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;
	
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	
	FHitResult Hit;
	GetWorld()->LineTraceSingleByObjectType(OUT Hit, PlayerViewPointLocation, LineTraceEnd, FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters);
	AActor* ActorHit = Hit.GetActor();
	if (ActorHit) {
		UE_LOG(LogTemp, Error, TEXT("Grab pressed %s"),(*ActorHit->GetName()));
	}
	return Hit;
};
