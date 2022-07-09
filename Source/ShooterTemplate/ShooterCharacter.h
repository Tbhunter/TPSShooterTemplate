// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTERTEMPLATE_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);

	/**
	* Called via input to turn at a given rate
	* @param Rate is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to look up/down at a given rate
	* @param Rate is a normalized rate, i.e. 1.0 means 100% of desired look up/down rate
	*/
	void LookUpAtRate(float Rate);

	/** Called when FireWeapon is pressed*/
	void FireWeapon();

	/** Weapon fire line tracing*/
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation);

	/** Character sprint functions*/
	void CharacterSprintPressed();
	void CharacterSprintReleased();

	/** Character aim functions */
	void AimingButtonPressed();
	void AimingButtonReleased();
    void ZoomInterpToFOV(float DeltaTime);
	
	/** Camera option functions */
	void ToggleCameraSide();

	

public:
	// Checks if character is dead
	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Take Damage to character
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	                         class AController* EventInstigator, AActor* DamageCauser) override;

private:
	/** Base Turn rate, in deg/sec. Other scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	/** Base look up rate, in deg/sec. Other scaling may affect final look up rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	/** Turn rate while not aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	/** Lookup rate while not aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	/** Turn rate while not aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;

	/** Lookup rate while not aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere)
	APlayerController* PlayerController;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(VisibleAnywhere)
	AWeapon* Weapon;

	UPROPERTY(EditDefaultsOnly)
	float MaxHealth{100};

	UPROPERTY(VisibleAnywhere)
	float Health;

	UPROPERTY(EditDefaultsOnly)
	float WalkSpeed{160.f};

	UPROPERTY(EditDefaultsOnly)
	float RunSpeed{330.f};

	/** Checks if the character is walking */
	UPROPERTY(VisibleAnywhere)
	bool bIsWalking;

	/** Randomize gunshot sound cue*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	/** Randomize gunshot impact sound cue*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	/** Flash spawned at BarrelSocket */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;

	/** Montage for firing weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	/** Particles spawned upon bullet impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	/** Smoke trail for bullets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	/** True when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/** Default camera field of view*/
	float CameraDefaultFOV;

	/** Field of view value when zoomed */
	float CameraZoomedFOV;

	/** Current field of view this frame */
	float CameraCurrentFOV;

	/** Interp speed for zooming when aiming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;
public:
	// Returns CameraBoom Component when called
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	// Returns the players FollowCamera when called
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	// Returns the players walk status
	FORCEINLINE bool GetIsWalking() const { return bIsWalking; }

	// Returns the players aim status
	FORCEINLINE bool GetIsAiming() const { return bAiming; }
};
