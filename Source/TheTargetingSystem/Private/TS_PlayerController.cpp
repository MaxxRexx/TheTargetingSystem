// Copyright 2023 Ibrahim Akinde. Discord: Ragnar#9805. Email: ibmaxx@hotmail.com. Website: https://www.ibrahimakinde.com . All Rights Reserved.


#include "TS_PlayerController.h"

#include "TargetingSystemComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

#define PRINT_SCREEN(Color, String) GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Color, FString::Printf(TEXT("%s"), *String))
//PRINT_SCREEN(White, OwningActor->GetName());


ATS_PlayerController::ATS_PlayerController()
{
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception Component"));
	TargetingSystemComponent = CreateDefaultSubobject<UTargetingSystemComponent>(TEXT("Targeting System Component"));
	SenseConfig_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Sense Configuration"));
	
	if (SenseConfig_Sight)
	{
		SenseConfig_Sight->SightRadius = 500;
		SenseConfig_Sight->LoseSightRadius = 600;
		SenseConfig_Sight->PeripheralVisionAngleDegrees = 180;
		SenseConfig_Sight->DetectionByAffiliation.bDetectEnemies = true;
		SenseConfig_Sight->DetectionByAffiliation.bDetectNeutrals = true;
		SenseConfig_Sight->DetectionByAffiliation.bDetectFriendlies = true;
		SenseConfig_Sight->SetMaxAge(1);
		
		AIPerceptionComponent->ConfigureSense(*SenseConfig_Sight);
		AIPerceptionComponent->SetDominantSense(SenseConfig_Sight->GetSenseImplementation());
	}
	
	AIPerceptionComponent->bEditableWhenInherited = true;
	GetTargetingSystemComponent()->bEditableWhenInherited = true;
}

void ATS_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindKey(EKeys::T, IE_Pressed, this, &ATS_PlayerController::TargetCenter);
	InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ATS_PlayerController::TargetLeft);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATS_PlayerController::TargetRight);
}

void ATS_PlayerController::TargetCenter()
{
	GetTargetingSystemComponent()->ServerLockOnToTarget();
}

void ATS_PlayerController::TargetLeft()
{
	GetTargetingSystemComponent()->ServerLockOnToTarget_Left();
}

void ATS_PlayerController::TargetRight()
{
	GetTargetingSystemComponent()->ServerLockOnToTarget_Right();
}

void ATS_PlayerController::BeginPlay()
{
	Super::BeginPlay();
	GetTargetingSystemComponent()->ServerInitializeVariables(GetPerceptionComponent(),GetPawn());
}
