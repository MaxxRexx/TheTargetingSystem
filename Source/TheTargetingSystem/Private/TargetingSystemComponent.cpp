// Copyright 2023 Ibrahim Akinde. Discord: Ragnar#9805. Email: ibmaxx@hotmail.com. Website: https://www.ibrahimakinde.com . All Rights Reserved.


#include "TargetingSystemComponent.h"

#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "TimerManager.h"

/* 

Debug Helpers

#define PRINT_SCREEN(Color, String) GEngine->AddOnScreenDebugMessage(-1, 20, FColor::Color, FString::Printf(TEXT("%s"), *String))

if (Cast<APlayerController>(OwningActor->GetInstigatorController()))
{
	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, FString::Printf(TEXT("%s"), *ActorToTarget->GetName()));
	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, FString::Printf(TEXT("%d"), AllPerceivedActors.Num()));
	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::White, FString::Printf(TEXT("%s"), *AllPerceivedActors[0]->GetName()));
	GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, FString::Printf(TEXT("%d"), bDoOnce_DetermineActorToTarget));
}

PRINT_SCREEN(White, OwningActor->GetName());

*/

// Set default values for this component's properties in constructor
UTargetingSystemComponent::UTargetingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

void UTargetingSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION (UTargetingSystemComponent, bInitialize, COND_InitialOnly);
	DOREPLIFETIME(UTargetingSystemComponent, OwningActor);
	DOREPLIFETIME(UTargetingSystemComponent, AIPerceptionComponent);
	DOREPLIFETIME(UTargetingSystemComponent, AllPerceivedActors);
	DOREPLIFETIME(UTargetingSystemComponent, LowestRotationAngle_Array);
	DOREPLIFETIME(UTargetingSystemComponent, ActorToTarget);
	DOREPLIFETIME(UTargetingSystemComponent, bIsLockOnEnabled);
	DOREPLIFETIME(UTargetingSystemComponent, bOpenGateForTick);
	DOREPLIFETIME(UTargetingSystemComponent, DelayBeforeLockOn);
	DOREPLIFETIME(UTargetingSystemComponent, ShouldShowWidgetForLockOn);
	DOREPLIFETIME(UTargetingSystemComponent, AutoSwitchTargetsOnPerceptionFailure);
	DOREPLIFETIME(UTargetingSystemComponent, AutoSwitchTargetsOnActorToTargetInvalid);
	DOREPLIFETIME(UTargetingSystemComponent, InterpolationSpeed);
}



// Called every frame - check if Lock-On gate can be opened
void UTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	EnterLockOnClientGate();
}





//.................................................................................................

//Get Angle Between Owning Character and Possible Actor To Target using Delta Rotation algorithm
float UTargetingSystemComponent::GetAngleBetweenVectors_UsingDeltaRotation(const FVector InStart, const FVector InTarget, const FRotator InRotationToCheckAgainst)
{
	const FRotator LookAtRotator = UKismetMathLibrary::FindLookAtRotation(InStart, InTarget);
	const FRotator DeltaRotatorResult =  UKismetMathLibrary::NormalizedDeltaRotator(LookAtRotator, InRotationToCheckAgainst);
	return DeltaRotatorResult.Yaw;
}

//Get Angle Between Owning Character and Possible Actor To Target using ArcTan2 Difference algorithm
float UTargetingSystemComponent::GetAngleBetweenVectors_UsingArcTanDifference(const FVector InStart, const FVector InTarget, const FVector InDirectionVector)
{
	const FRotator LookAtRotator = UKismetMathLibrary::FindLookAtRotation(InStart, InTarget);
	const FVector ForwardVectorFromLookAtRotator = UKismetMathLibrary::GetForwardVector(LookAtRotator);

	const float InverseTanA = UKismetMathLibrary::Atan2(InDirectionVector.X, InDirectionVector.Y);
	const float InverseTanB = UKismetMathLibrary::Atan2(ForwardVectorFromLookAtRotator.X, ForwardVectorFromLookAtRotator.Y);

	return  InverseTanA - InverseTanB;
}

//.................................................................................................



//Sort Function....................................................................................

TArray<float> UTargetingSystemComponent::SortArray_Float(const TArray<float> &ArrayToSort, const bool bReversed) const
{
	TArray<float> LocalArray_Out;
	TArray<float> LocalArray_Copy_1 = ArrayToSort;

	if (LocalArray_Out.Num() > 0)
	{
		LocalArray_Out.Empty();
	}
	
	if (bReversed)
	{
		TArray<float> LocalArray_Copy_2 = ArrayToSort;

		Algo::Reverse(LocalArray_Copy_2);

		int MinIndex;
		float MinValue;

		for (int i = 0; i < LocalArray_Copy_2.Num() - 1; i++)
		{
			UKismetMathLibrary::MinOfFloatArray(LocalArray_Copy_1, MinIndex, MinValue);
			SetArrayElement<float>(LocalArray_Out, MinValue, i, true);
			LocalArray_Copy_1.RemoveAt(MinIndex);
		}
	}
	else
	{
		int MinIndex;
		float MinValue;
		
		for (int i = 0; i < LocalArray_Copy_1.Num(); i++)
		{
			UKismetMathLibrary::MinOfFloatArray(LocalArray_Copy_1, MinIndex, MinValue);
			SetArrayElement<float>(LocalArray_Out, MinValue, i, true);
			LocalArray_Copy_1.RemoveAt(MinIndex);
		}
	}
	
	return LocalArray_Out;
}

//.................................................................................................



//Helper Functions --------------------------------------------------------------------------------
void UTargetingSystemComponent::FindIdealActorToTarget(TArray<float> &InArray)
{
	for (int i = 0; i < AllPerceivedActors.Num(); i++)
	{
		if (AllPerceivedActors[i])
		{
			if (bUseActorRotation)
			{
				if (bUseDeltaRotation)
				{
					if (!InArray.Contains(abs(GetAngleBetweenVectors_UsingDeltaRotation(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), OwningActor->GetActorRotation()))))
					{
						const float MinValue = abs(GetAngleBetweenVectors_UsingDeltaRotation(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), OwningActor->GetActorRotation()));
						SetArrayElement<float>(InArray, MinValue , i, true);
					}
				}
				else
				{
					if (!InArray.Contains(abs(GetAngleBetweenVectors_UsingArcTanDifference(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), OwningActor->GetActorForwardVector()))))
					{
						const float MinValue = abs(GetAngleBetweenVectors_UsingArcTanDifference(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), OwningActor->GetActorForwardVector()));
						SetArrayElement<float>(InArray, MinValue , i, true);
					}
				}
			}
			else
			{
				const ACharacter* CharacterActor = nullptr;
					
				if (bUseDeltaRotation)
				{
					if (Cast<ACharacter>(OwningActor))
					{
						CharacterActor = Cast<ACharacter>(OwningActor);

						if (!InArray.Contains(abs(GetAngleBetweenVectors_UsingDeltaRotation(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), CharacterActor->GetControlRotation()))))
						{
							const float MinValue = (abs(GetAngleBetweenVectors_UsingDeltaRotation(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), CharacterActor->GetControlRotation())));
							SetArrayElement<float>(InArray, MinValue , i, true);
						}
					}
				}
				else
				{
					if (!InArray.Contains(abs(GetAngleBetweenVectors_UsingArcTanDifference(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), CharacterActor->GetActorForwardVector()))))
					{
						const float MinValue = (abs(GetAngleBetweenVectors_UsingArcTanDifference(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), CharacterActor->GetActorForwardVector())));
						SetArrayElement<float>(InArray, MinValue , i, true);
					}
				}
			}
		}
	}
}

void UTargetingSystemComponent::FindIdealActorToTarget_NoAbs(TArray<float> &InArray)
{
	for (int i = 0; i < AllPerceivedActors.Num(); i++)
	{
		if (AllPerceivedActors[i])
		{
			if (bUseActorRotation)
			{
				if (bUseDeltaRotation)
				{
					if (!InArray.Contains(GetAngleBetweenVectors_UsingDeltaRotation(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), OwningActor->GetActorRotation())))
					{
						const float MinValue = GetAngleBetweenVectors_UsingDeltaRotation(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), OwningActor->GetActorRotation());
						SetArrayElement<float>(InArray, MinValue , i, true);
					}
				}
				else
				{
					if (!InArray.Contains(GetAngleBetweenVectors_UsingArcTanDifference(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), OwningActor->GetActorForwardVector())))
					{
						const float MinValue = GetAngleBetweenVectors_UsingArcTanDifference(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), OwningActor->GetActorForwardVector());
						SetArrayElement<float>(InArray, MinValue , i, true);
					}
				}
			}
			else
			{
				const ACharacter* CharacterActor = nullptr;
					
				if (bUseDeltaRotation)
				{
					if (Cast<ACharacter>(OwningActor))
					{
						CharacterActor = Cast<ACharacter>(OwningActor);

						if (!InArray.Contains(GetAngleBetweenVectors_UsingDeltaRotation(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), CharacterActor->GetControlRotation())))
						{
							const float MinValue = GetAngleBetweenVectors_UsingDeltaRotation(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), CharacterActor->GetControlRotation());
							SetArrayElement<float>(InArray, MinValue , i, true);
						}
					}
				}
				else
				{
					if (!InArray.Contains(GetAngleBetweenVectors_UsingArcTanDifference(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), CharacterActor->GetActorForwardVector())))
					{
						const float MinValue = GetAngleBetweenVectors_UsingArcTanDifference(OwningActor->GetActorLocation(), AllPerceivedActors[i]->GetActorLocation(), CharacterActor->GetActorForwardVector());
						SetArrayElement<float>(InArray, MinValue , i, true);
					}
				}
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------


//initialize variables
void UTargetingSystemComponent::ServerInitializeVariables_Implementation(UAIPerceptionComponent* InAIPerceptionComponent,
                                                                   AActor* InOwningCharacter)
{
	if (bInitialize)
	{
		OwningActor = InOwningCharacter;
		AIPerceptionComponent = InAIPerceptionComponent;
		
		if (AIPerceptionComponent && AIPerceptionComponent->GetDominantSense())
		{
			SenseToUseForPerception = AIPerceptionComponent->GetDominantSense();
		}
		
		ServerStorePerceivedActorsInArray();
	}
}

bool UTargetingSystemComponent::ServerInitializeVariables_Validate(UAIPerceptionComponent* InAIPerceptionComponent,
	AActor* InOwningCharacter)
{
	return true;
}





//Store Perceived Actors In Array|A timer event is called here, and it runs on an interval and a loop condition.
//Every interval, it stores actors currently being perceived by the AI Perception Component
void UTargetingSystemComponent::ServerStorePerceivedActorsInArray_Implementation()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_StorePerceivedActors, this, &UTargetingSystemComponent::Timer_StorePerceivedActorInArray, TimerIntervalForStoringPerceivedActorsInArray, bLoopTimerForStoringPerceivedActorsInArray);
}

bool UTargetingSystemComponent::ServerStorePerceivedActorsInArray_Validate()
{
	return true;
}



void UTargetingSystemComponent::ServerResetAllArrays_Implementation()
{
	if (ActorToTarget)
	{
		ClientHideOrDisplayWidgetForLockedOnTarget(false, false);
	}
	AllPerceivedActors.Empty();
	LowestRotationAngle_Array.Empty();
	LowestRotationAngle_Left_Array.Empty();
	LowestRotationAngle_Left_Array_Sorted.Empty();
	LowestRotationAngle_Right_Array.Empty();
	LowestRotationAngle_Right_Array_Sorted.Empty();
	ActorToTarget = nullptr;
	ServerEnableOrDisableLockOn(false);
}

bool UTargetingSystemComponent::ServerResetAllArrays_Validate()
{
	return true;
}




void UTargetingSystemComponent::Timer_StorePerceivedActorInArray()
{
	if (AIPerceptionComponent)
	{
		TArray<AActor*> ActorsCurrentlyPerceived;
		
		if (SenseToUseForPerception)
		{
			AIPerceptionComponent->GetCurrentlyPerceivedActors(SenseToUseForPerception, ActorsCurrentlyPerceived);
		}
		else
		{
			const TSubclassOf<UAISense_Sight> SightSenseToUse;
			AIPerceptionComponent->GetCurrentlyPerceivedActors(SightSenseToUse, ActorsCurrentlyPerceived);
		}

		if (ActorsCurrentlyPerceived.Num() > 0)
		{
			if (bFilterPerceivedActorsArray && ClassToFilterPerceivedActorsTo != nullptr)
			{
				TArray<AActor*> FilteredArray_ActorsCurrentlyPerceived;
				
				UKismetArrayLibrary::FilterArray(ActorsCurrentlyPerceived, ClassToFilterPerceivedActorsTo, FilteredArray_ActorsCurrentlyPerceived);
				
				 for (AActor* FilteredActorPerceived : FilteredArray_ActorsCurrentlyPerceived)
				 {
					 if (FilteredActorPerceived && !AllPerceivedActors.Contains(FilteredActorPerceived))
					 {
					 	AllPerceivedActors.Add(FilteredActorPerceived);
					 }
				 }
			}
			else
			{
				for (AActor* ActorPerceived : ActorsCurrentlyPerceived)
				{
					if (ActorPerceived && !AllPerceivedActors.Contains(ActorPerceived))
					{
						AllPerceivedActors.Add(ActorPerceived);
					}
				}
			}

			ServerDetermineActorWithTheLeastActorRotationAngleRequiredToTarget();
		}
		else
		{
			ClearTimer_StorePerceivedActorsInArray();
		}
	}
	else
	{
		ClearTimer_StorePerceivedActorsInArray();
	}
	
}

void UTargetingSystemComponent::ClearTimer_StorePerceivedActorsInArray()
{
	//GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StorePerceivedActors);
	ServerResetAllArrays();
}




//Determine Actor With The Least Actor Rotation Angle Required To Target
//Here, we check for the best possible valid actor to target within our perception range with the following condition:
//the best actor to target is the actor that requires the least amount of our actor's (character's) rotation to face that actor.
void UTargetingSystemComponent::ServerDetermineActorWithTheLeastActorRotationAngleRequiredToTarget_Implementation()
{
	if (OwningActor && AllPerceivedActors.Num() > 0)
	{
		FindIdealActorToTarget(LowestRotationAngle_Array);
		
		ServerDetermineActorToTargetFromLowestRotationAngleArray();
	}
	else
	{
		ClearTimer_StorePerceivedActorsInArray();
	}
	
}

bool UTargetingSystemComponent::ServerDetermineActorWithTheLeastActorRotationAngleRequiredToTarget_Validate()
{
	return true;
}





//Determine Actor To Target From Lowest Rotation Angle Array
void UTargetingSystemComponent::ServerDetermineActorToTargetFromLowestRotationAngleArray_Implementation()
{
	if (bDoOnce_DetermineActorToTarget)
	{
		bDoOnce_DetermineActorToTarget = false;
		
		if (LowestRotationAngle_Array.Num() > 1)
		{
			int MinIndex;
			float MinValue;
			
			UKismetMathLibrary::MinOfFloatArray(LowestRotationAngle_Array, MinIndex, MinValue);
			
			if (AllPerceivedActors.IsValidIndex(MinIndex))
			{
				ActorToTarget = AllPerceivedActors[MinIndex];
			}
		}
		else if (LowestRotationAngle_Array.Num() == 1)
		{
			if (AllPerceivedActors.IsValidIndex(0))
			{
				ActorToTarget = AllPerceivedActors[0];
			}
		}
	}
}

bool UTargetingSystemComponent::ServerDetermineActorToTargetFromLowestRotationAngleArray_Validate()
{
	return true;
}



void UTargetingSystemComponent::ClientHideOrDisplayWidgetForLockedOnTarget_Implementation(const bool bShow, const bool bUseDelay)
{
	if (bShow)
	{
		if (bUseDelay)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_DelayBeforeLockOn, this, &UTargetingSystemComponent::ClientShowWidgetForTargetNotBeingLockOnTo, DelayBeforeLockOn, false, -1);
		}
		else
		{
			ClientShowWidgetForTargetNotBeingLockOnTo();
		}
	}
	else
	{
		if (bUseDelay)
		{
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_DelayBeforeLockOn, this, &UTargetingSystemComponent::ClientHideWidgetForTargetNotBeingLockOnTo, DelayBeforeLockOn, false, -1);
		}
		else
		{
			ClientHideWidgetForTargetNotBeingLockOnTo();
		}
	}
}




void UTargetingSystemComponent::ClientShowWidgetForTargetNotBeingLockOnTo_Implementation() const
{
	if (ShouldShowWidgetForLockOn && ActorToTarget)
	{
		if (UWidgetComponent *WidgetComponent = ActorToTarget->FindComponentByClass<UWidgetComponent>())
		{
				WidgetComponent->SetHiddenInGame(false);
		}
	}
}

void UTargetingSystemComponent::ClientHideWidgetForTargetNotBeingLockOnTo_Implementation() const
{
	if (ShouldShowWidgetForLockOn && ActorToTarget)
	{
		if (UWidgetComponent *WidgetComponent = ActorToTarget->FindComponentByClass<UWidgetComponent>())
		{
			WidgetComponent->SetHiddenInGame(true);
		}
	}
}




void UTargetingSystemComponent::ServerSwitchTargets_Implementation(const bool bSwitchTarget)
{
	if (!bSwitchTarget)
	{
		bDoOnce_SwitchTargets = true;
	}

	if (bSwitchTarget && bDoOnce_SwitchTargets)
	{
		bDoOnce_SwitchTargets = false;
		int MinIndex;
		float MinValue;
		UKismetMathLibrary::MinOfFloatArray(LowestRotationAngle_Array, MinIndex, MinValue);

		if (AllPerceivedActors.IsValidIndex(MinIndex))
		{
			ActorToTarget = AllPerceivedActors[MinIndex];
			bCanSwitchTargets = true;
		}
		else
		{
			bDoOnce_SwitchTargets = true;
		}
	}
}

bool UTargetingSystemComponent::ServerSwitchTargets_Validate(bool bSwitchTarget)
{
	return true;
}




void UTargetingSystemComponent::ServerSwitchToTheTargetToTheLeftOfCurrentTarget_Implementation()
{
	LowestRotationAngle_Left_Array_Sorted.Empty();
	
	FindIdealActorToTarget_NoAbs(LowestRotationAngle_Left_Array);
	
	for (float AngleLeft : LowestRotationAngle_Left_Array)
	{
		if (!UKismetMathLibrary::NearlyEqual_FloatFloat(abs(AngleLeft), 0, ErrorTolerance_Left))
		{
			LowestRotationAngle_Left_Array_Sorted.AddUnique(AngleLeft);
		}
	}
	
	 if (LowestRotationAngle_Left_Array_Sorted.Num() > 0)
	{
		//Sort Array For All Items Less Than 0 (Negative) In Descending Order
		LowestRotationAngle_Left_Array_Sorted = SortArray_Float(LowestRotationAngle_Left_Array_Sorted, true);
	 	
		if (LowestRotationAngle_Left_Array_Sorted.Num() > 0)
		{
			if (LowestRotationAngle_Left_Array_Sorted[0])
            {
            	if (LowestRotationAngle_Left_Array.Num() > 0)
            	{
            		if (AllPerceivedActors.Num() > 0)
            		{
            			if (AllPerceivedActors[LowestRotationAngle_Left_Array.Find(LowestRotationAngle_Left_Array_Sorted[0])])
            			{
            				ClientHideOrDisplayWidgetForLockedOnTarget(false, false);
            	
            				ActorToTarget = AllPerceivedActors[LowestRotationAngle_Left_Array.Find(LowestRotationAngle_Left_Array_Sorted[0])];
    
            				ClientHideOrDisplayWidgetForLockedOnTarget(true, true);
            			}
            		}
            	}
            }
		}
		else
		{
			ServerLockOnToTarget_Right();
		}
	}
	 else
	 {
	 	ServerSwitchToTheTargetToTheRightOfCurrentTarget();
	 }
}

bool UTargetingSystemComponent::ServerSwitchToTheTargetToTheLeftOfCurrentTarget_Validate()
{
	return true;
}




void UTargetingSystemComponent::ServerSwitchToTheTargetToTheRightOfCurrentTarget_Implementation()
{
	LowestRotationAngle_Right_Array_Sorted.Empty();
	
	FindIdealActorToTarget(LowestRotationAngle_Right_Array);

	for (float AngleRight : LowestRotationAngle_Right_Array)
	{
		if (AngleRight > 0)
		{
			if (!UKismetMathLibrary::NearlyEqual_FloatFloat(abs(AngleRight), 0, ErrorTolerance_Right))
			{
				LowestRotationAngle_Right_Array_Sorted.AddUnique(AngleRight);
			}
		}
	}

	if (LowestRotationAngle_Right_Array_Sorted.Num() > 0)
	{
		//Sort Array For All Items Greater Than 0 (Positive) In Ascending Order
		LowestRotationAngle_Right_Array_Sorted = SortArray_Float(LowestRotationAngle_Right_Array_Sorted, false);

		
		if (AllPerceivedActors[LowestRotationAngle_Right_Array.Find(LowestRotationAngle_Right_Array_Sorted[0])])
		{
			ClientHideOrDisplayWidgetForLockedOnTarget(false, false);
			
			ActorToTarget = AllPerceivedActors[LowestRotationAngle_Right_Array.Find(LowestRotationAngle_Right_Array_Sorted[0])];

			ClientHideOrDisplayWidgetForLockedOnTarget(true, true);
		}
	}
}

bool UTargetingSystemComponent::ServerSwitchToTheTargetToTheRightOfCurrentTarget_Validate()
{
	return true;
}




void UTargetingSystemComponent::ServerEnableOrDisableLockOn_Implementation(const bool bInEnable)
{
	bIsLockOnEnabled = bInEnable;
	if (!bIsLockOnEnabled)
	{
		bDoOnce_DetermineActorToTarget = true;
	}
}

bool UTargetingSystemComponent::ServerEnableOrDisableLockOn_Validate(bool bInEnable)
{
	return true;
}



//Lock On To Target
void UTargetingSystemComponent::ServerLockOnToTarget_Implementation()
{
	if (bCanLockOn)
	{
		if (bIsLockOnEnabled)
		{
			ServerCloseGateForTick();
			ServerEnableOrDisableLockOn(false);
			ClientHideOrDisplayWidgetForLockedOnTarget(false, true);
		}
		else //if (OwningActor->GetRemoteRole() == ROLE_Authority)
		{
			ServerEnableOrDisableLockOn(true);
			
			FTimerHandle TimerHandle_DelayBeforeOpeningGate;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_DelayBeforeOpeningGate, this, &UTargetingSystemComponent::MulticastOpenGateForTick, DelayBeforeLockOn, false, -1);
			ClientHideOrDisplayWidgetForLockedOnTarget(true, true);
		}
	}
}

bool UTargetingSystemComponent::ServerLockOnToTarget_Validate()
{
	return true;
}



//Lock On To Target To The Left
void UTargetingSystemComponent::ServerLockOnToTarget_Left_Implementation()
{
	if (OwningActor && bCanLockOn && bIsLockOnEnabled)
	{
		ServerSwitchToTheTargetToTheLeftOfCurrentTarget();
	}
}

bool UTargetingSystemComponent::ServerLockOnToTarget_Left_Validate()
{
	return true;
}



//Lock On To Target To The Right
void UTargetingSystemComponent::ServerLockOnToTarget_Right_Implementation()
{
	if (OwningActor && bCanLockOn && bIsLockOnEnabled)
	{
		ServerSwitchToTheTargetToTheRightOfCurrentTarget();
	}
}

bool UTargetingSystemComponent::ServerLockOnToTarget_Right_Validate()
{
	return true;
}





//Enter Lock-On Gate_Client|Make Sure Owning Actor Is A Valid Character Before Opening Gate
void UTargetingSystemComponent::EnterLockOnClientGate()
{
	if (IsValid(Cast<ACharacter>(OwningActor)))
	{
		if (bOpenGateForTick)
		{
			if (ActorToTarget)
			{
				if (AllPerceivedActors.Contains(ActorToTarget))
				{
					const ACharacter* OwningCharacter = Cast<ACharacter>(OwningActor);

					const float ControllerPitch = OwningCharacter->GetController()->GetControlRotation().Pitch;
					const float ControllerRoll = OwningCharacter->GetController()->GetControlRotation().Roll;

					//Find and Interpolate between Owning Character's Location and Target's Location
					const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(OwningActor->GetActorLocation(), ActorToTarget->GetActorLocation());

					const FRotator InterpolationRotation = UKismetMathLibrary::RInterpTo(OwningCharacter->GetController()->GetControlRotation(), LookAtRotation, GetWorld()->DeltaTimeSeconds, InterpolationSpeed);

					const float ControllerYaw = InterpolationRotation.Yaw;

					const FRotator InRot = UKismetMathLibrary::MakeRotator(ControllerRoll, ControllerPitch, ControllerYaw);
					
					OwningCharacter->GetController()->SetControlRotation(InRot);
				}
				
				else if (AutoSwitchTargetsOnPerceptionFailure)
				{
					ServerCloseGateForTick();
					
					if (bCanSwitchTargets)
					{
						bCanSwitchTargets = false;
						ServerSwitchTargets(true);
						ServerLockOnToTarget();
					}
				}
				else
				{
					ServerCloseGateForTick();
				}
			}
			else if (AutoSwitchTargetsOnActorToTargetInvalid)
			{
				ServerCloseGateForTick();
				
				if (bCanSwitchTargets)
				{
					bCanSwitchTargets = false;
					ServerSwitchTargets(true);
					ServerLockOnToTarget();
				}
			}
			else
			{
				ServerCloseGateForTick();
			}
		}
		else
		{
			if (Cast<APlayerController>(OwningActor->GetInstigatorController()))
			{
				UE_LOG(LogTemp, Warning, TEXT("%s"), *OwningActor->GetName());
			}
			ServerCloseGateForTick();
		}
	}
}



void UTargetingSystemComponent::ServerCloseGateForTick_Implementation()
{
	MulticastCloseGateForTick();
}

bool UTargetingSystemComponent::ServerCloseGateForTick_Validate()
{
	return true;
}


void UTargetingSystemComponent::MulticastCloseGateForTick_Implementation()
{
	bOpenGateForTick = false;
	//HideWidgetForTargetNotBeingLockOnTo();
}

bool UTargetingSystemComponent::MulticastCloseGateForTick_Validate()
{
	return true;
}


void UTargetingSystemComponent::MulticastOpenGateForTick_Implementation()
{
	bOpenGateForTick = true;
}

bool UTargetingSystemComponent::MulticastOpenGateForTick_Validate()
{
	return true;
}