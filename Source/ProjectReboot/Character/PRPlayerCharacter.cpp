// Fill out your copyright notice in the Description page of Project Settings.


#include "PRPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "RogueliteAbilityHandlerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/Input/PREnhancedInputComponent.h"
#include "ProjectReboot/UI/Crosshair/PRCrosshairViewModel.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"


// Sets default values
APRPlayerCharacter::APRPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	
	AbilitySystem = CreateDefaultSubobject<UPRAbilitySystemComponent>(TEXT("AbilitySystem"));
	CommonAttributeSet = CreateDefaultSubobject<UPRCommonAttributeSet>(TEXT("CommonAttributeSet"));
	WeaponAttributeSet = CreateDefaultSubobject<UPRWeaponAttributeSet>(TEXT("WeaponAttributeSet"));
	RogueliteAbilityHandler = CreateDefaultSubobject<URogueliteAbilityHandlerComponent>(TEXT("RogueliteAbilityHandler"));
	EquipmentManager = CreateDefaultSubobject<UPREquipmentManagerComponent>(TEXT("EquipmentManager"));
}

UAbilitySystemComponent* APRPlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

bool APRPlayerCharacter::IsCrouching() const
{
	if (AbilitySystem)
	{
		return AbilitySystem->HasMatchingGameplayTag(TAG_State_Crouch);
	}
	return false;
}

bool APRPlayerCharacter::IsSprinting() const
{
	if (AbilitySystem)
	{
		return AbilitySystem->HasMatchingGameplayTag(TAG_State_Sprint);
	}
	return false;
}

void APRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystem)
	{
		AbilitySystem->GiveAbilitySet(DefaultAbilitySet, DefaultAbilitySetHandles);
	}
}

// Called to bind functionality to input
void APRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UPREnhancedInputComponent* EnhancedInputComponent = Cast<UPREnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		
		if (InputConfig)
		{
			EnhancedInputComponent->BindInputActionsByConfig(InputConfig, this, &ThisClass::OnTaggedInputPressed, &ThisClass::OnTaggedInputReleased);
		}
	}
}

void APRPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void APRPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (AbilitySystem)
	{
		AbilitySystem->InitAbilityActorInfo(this, this);
	}

	BindCrosshairViewModel();
}

void APRPlayerCharacter::UnPossessed()
{
	UnbindCrosshairViewModel();
	Super::UnPossessed();
}

void APRPlayerCharacter::OnTaggedInputPressed(FGameplayTag InputTag)
{
	if (AbilitySystem)
	{
		AbilitySystem->AbilityInputPressed(InputTag);
	}
}

void APRPlayerCharacter::OnTaggedInputReleased(FGameplayTag InputTag)
{
	if (AbilitySystem)
	{
		AbilitySystem->AbilityInputReleased(InputTag);
	}
}

void APRPlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APRPlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DesiredLookDirection = LookAxisVector.X;

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

UPRCrosshairViewModel* APRPlayerCharacter::GetCrosshairViewModel() const
{
	APlayerController* PC = GetController<APlayerController>();
	if (!PC)
	{
		return nullptr;
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return nullptr;
	}

	UPRViewModelSubsystem* VMS = LP->GetSubsystem<UPRViewModelSubsystem>();
	if (!VMS)
	{
		return nullptr;
	}

	return VMS->GetOrCreateGlobalViewModel<UPRCrosshairViewModel>();
}

void APRPlayerCharacter::BindCrosshairViewModel()
{
	if (UPRCrosshairViewModel* VM = GetCrosshairViewModel())
	{
		VM->BindToASC(AbilitySystem);
	}
}

void APRPlayerCharacter::UnbindCrosshairViewModel()
{
	if (UPRCrosshairViewModel* VM = GetCrosshairViewModel())
	{
		VM->UnbindFromASC();
	}
}
