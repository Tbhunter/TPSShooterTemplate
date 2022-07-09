// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

#include "DrawDebugHelpers.h"
#include "ShooterTemplateGameModeBase.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	BaseTurnRate(65.f),
	BaseLookUpRate(45.f),
	bIsWalking(true),
	bAiming(false),
	CameraDefaultFOV(0.f), // set in BeginPlay
	CameraZoomedFOV(50.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(10.f)


{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creates camera boom (pulls in towards character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 110.f; // camera follows at this distance behind character
	CameraBoom->bUsePawnControlRotation = true; // rotate arm based on controller
	CameraBoom->SocketOffset = FVector(0, 50.f, 70.f);

	// Create Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // attach camera to booms end
	FollowCamera->bUsePawnControlRotation = false; // camera does not rotate relative to boom

	// Dont rotate when the controller rotates, Let controller only affect the camera
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	bUseControllerRotationPitch = false;

	//Configure Character Movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // character moves in dir of input, not camera
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->RotationRate = FRotator(0, 250.0f, 0);
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	// Set player camera pitch range
	PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController)
	{
		// clamps camera to certain range
		PlayerController->PlayerCameraManager->ViewPitchMin = -60.0;
		PlayerController->PlayerCameraManager->ViewPitchMax = 60.0;
	}

	Health = MaxHealth;
	// Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass);
	// GetMesh()->HideBoneByName(TEXT("weapon_r"), EPhysBodyOp::PBO_None);
	// Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform,TEXT("WeaponSocket"));
	// Weapon->SetOwner(this);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ZoomInterpToFOV(DeltaTime);
}

/**==============================================================================
 * ==============================================================================*/

#pragma region Character Input From Player
// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	/**
	* Character movement axis mappings - Movement
	* Directional Movement / Turn at Rates / Mouse Input  (WASD + Mouse)
	*/
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"), this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis(TEXT("TurnRate"), this, &AShooterCharacter::TurnAtRate);

	// Mouse Input Bindings
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);

	/**
	 * Character Action Bindings-One off button presses. Not movement
	 * Camera Toggle / Weapon Fire / Running / Aiming
	 */
	PlayerInputComponent->BindAction(TEXT("CameraSwitchSides"), EInputEvent::IE_Pressed, this,
	                                 &AShooterCharacter::ToggleCameraSide);
	PlayerInputComponent->BindAction(TEXT("FireButton"), EInputEvent::IE_Pressed, this, &AShooterCharacter::FireWeapon);
	PlayerInputComponent->BindAction(TEXT("Run"), EInputEvent::IE_Pressed, this,
	                                 &AShooterCharacter::CharacterSprintPressed);
	PlayerInputComponent->BindAction(TEXT("Run"), EInputEvent::IE_Released, this,
	                                 &AShooterCharacter::CharacterSprintReleased);
	PlayerInputComponent->BindAction(TEXT("AimButton"), EInputEvent::IE_Pressed, this,
	                                 &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction(TEXT("AimButton"), EInputEvent::IE_Released, this,
	                                 &AShooterCharacter::AimingButtonReleased);


	/** Character Jump functionality can be added by uncommenting the following lines of code
	 *  WARNING: Enabling the jump feature may allow unintended scenarios within the gameplay
	 */
	//PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	//PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Released, this, &ACharacter::StopJumping);
}
#pragma endregion
/**==============================================================================
 * ==============================================================================*/

#pragma region Character Combat Functionality

float AShooterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                    AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	DamageToApply = FMath::Min(Health, DamageToApply);
	Health -= DamageToApply;
	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health);

	if (IsDead())
	{
		AShooterTemplateGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AShooterTemplateGameModeBase>();
		if (GameMode != nullptr)
		{
			GameMode->PawnKilled(this);
		}
		DetachFromControllerPendingDestroy();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	return DamageToApply;
}

void AShooterCharacter::FireWeapon()
{
	if (!bIsWalking) // character can only fire if he is walking
	{
		return;
	}

	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	FVector BeamEndPoint;
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);

		if (bBeamEnd)
		{
			// Spawn impact particles after updating beam end point
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamEnd);
			}

			// Spawn bullet smoke beam particles
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(), BeamParticles, SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
	//Weapon->PullTrigger();
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	// Get current viewport size
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPos;
	FVector CrosshairWorldDir;

	// Get World pos and dir of crosshairs
	// TODO: This references a single player. possible change if multiplayer
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
	                                                               CrosshairLocation,
	                                                               CrosshairWorldPos,
	                                                               CrosshairWorldDir);

	// Was deprojection successful
	if (bScreenToWorld)
	{
		FHitResult ScreenTraceHit;
		const FVector Start{CrosshairWorldPos};
		const FVector End{CrosshairWorldPos + CrosshairWorldDir * 50'000.f};

		// Set OutBeamLocation to line trace end point
		OutBeamLocation = End;
		// Trace outward from crosshairs location
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECollisionChannel::ECC_Visibility);

		// Was their a trace hit?
		if (ScreenTraceHit.bBlockingHit)
		{
			// Beam end point now trace hit location
			OutBeamLocation = ScreenTraceHit.Location;
		}

		// Perform second trace from gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart{MuzzleSocketLocation};
		const FVector WeaponTraceEnd{OutBeamLocation};

		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd,
		                                     ECollisionChannel::ECC_Visibility);
		if (WeaponTraceHit.bBlockingHit) // object between barrel and end point
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
}


bool AShooterCharacter::IsDead() const
{
	return Health <= 0;
}

#pragma endregion
/**==============================================================================
 * ==============================================================================*/

#pragma region Character Input Functionality

void AShooterCharacter::MoveForward(float AxisValue)
{
	if ((Controller != nullptr) && AxisValue != 0.0f)
	{
		// find fright direction
		const FRotator Rotation{Controller->GetControlRotation()}; // Getting rotation info, from the char. controller
		const FRotator YawRotation{0.f, Rotation.Yaw, 0.f}; // Specifying Yaw rotation from the rotation info.

		// will get the y axis direction from a rotation matrix
		// Direction points in direction that corresponds to controllers orientation
		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X)};
		AddMovementInput(Direction, AxisValue);
	}
}

void AShooterCharacter::MoveRight(float AxisValue)
{
	if ((Controller != nullptr) && AxisValue != 0.0f)
	{
		// find right direction
		const FRotator Rotation{Controller->GetControlRotation()}; // Getting rotation info, from the char. controller
		const FRotator YawRotation{0, Rotation.Yaw, 0}; // Specifying Yaw rotation from the rotation info.

		// will get the y axis direction from a rotation matrix
		// Direction points in direction that corresponds to controllers orientation
		const FVector Direction{FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y)};
		AddMovementInput(Direction, AxisValue);
	}
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); // deg/sec * sec/frame
}

void AShooterCharacter::CharacterSprintPressed()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	bIsWalking = false;
}

void AShooterCharacter::CharacterSprintReleased()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	bIsWalking = true;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased()
{
	bAiming = false;
}

void AShooterCharacter::ZoomInterpToFOV(float DeltaTime)
{
	// Set current FOV
	if (bAiming && bIsWalking)
	{
		// Interp to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		// Interp to default FOV
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::ToggleCameraSide()
{
	// Changes whether or not the camera is on the left or ride side of character
	CameraBoom->SocketOffset.Y = -CameraBoom->SocketOffset.Y;
}

#pragma endregion
/**==============================================================================
 * ==============================================================================*/
