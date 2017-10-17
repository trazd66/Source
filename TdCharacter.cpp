// As a beginner who downloaded UE4 less than two weeks, I had a lot of trouble finding the right solution
// of how to rotate a character so that it faces the cursor(in a top down game), I feel like such basic operation should not 
// be a struggle to do. But anyway it may just be me lacking the google skill... Enjoy!

#include "TdCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"

ATdCharacter::ATdCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Rotate character to moving direction
	GetCharacterMovement()->bUseControllerDesiredRotation = true; //two events are mutually exclusive
	
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;


	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->RelativeRotation = FRotator(-60.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void ATdCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetCursorDir();//the method will be called every frame.
}

void ATdCharacter::SetCursorDir() {
	//note that the line 58-59 is seen pretty common on the internet to find the cursor porjection
	//onto the internet, however, they do not give me the right vector.
	//FVector CursorWorldLocation, CursorWorldDirection;
	//GetWorld()->GetFirstPlayerController()->DeprojectMousePositionToWorld(CursorWorldLocation, CursorWorldDirection);
	//=====================================================================//

	//gets location of the actor in the world
	FVector CurrLoc = this->GetActorLocation();

	// the right method of getting cursor location!!! note: i used the exact same method in the Epic Top Down Blueprint.
	FHitResult hitResult;
	GetWorld()->GetFirstPlayerController()->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true,hitResult);
	FVector hitLoc = hitResult.Location;

	//geting the original rotation of the acter;
	FRotator newRot = this->GetActorRotation();
	//Using atan2 function to get the degree between our character location and cursor location
	// aka how much we want the character to rotate
	float newYaw = (hitLoc - CurrLoc).Rotation().Yaw;;
	//Using that degree as the Yaw(rotating around Z axis) of our Frotator
	newRot.Yaw = newYaw;

	//in the end, we set it;
	this->GetController()->SetControlRotation(newRot);
}


void ATdCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &ATdCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ATdCharacter::MoveRight);


	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ATdCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ATdCharacter::LookUpAtRate);

}

void ATdCharacter::MoveForward(float value)
{
	if ((Controller != NULL) && (value != 0.0f))
	{
		// get forward vector
		const FVector Direction = GetActorForwardVector();
		AddMovementInput(Direction, value);
	}
}

void ATdCharacter::MoveRight(float value)
{
	if ((Controller != NULL) && (value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get Right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}
}



void ATdCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATdCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}