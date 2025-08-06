// Copyright 2023 Ibrahim Akinde. Discord: Ragnar#9805. Email: ibmaxx@hotmail.com. Website: https://www.ibrahimakinde.com . All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TargetingSystemComponent.generated.h"



//forward declarations
class UAIPerceptionComponent;
class UAISense;
class UWidgetComponent;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class THETARGETINGSYSTEM_API UTargetingSystemComponent : public UActorComponent
{
	GENERATED_BODY()
	
	// private variables
	bool bDoOnce_DetermineActorToTarget = true;
	bool bDoOnce_SwitchTargets = true;
	bool bCanSwitchTargets = false;
	bool bCanLockOn = true;
	
	//Find Ideal Actor To Target in given Array
	UFUNCTION(Category = HelperFunctions)
	void FindIdealActorToTarget(TArray<float> &InArray);

	UFUNCTION(Category = HelperFunctions)
	void FindIdealActorToTarget_NoAbs(TArray<float> &InArray);
	
	public:	
	// Sets default values for this component's properties
	UTargetingSystemComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	
	//Get Angle between vectors
	UFUNCTION(BlueprintGetter = "Get Angle Between Vectors")
	static float GetAngleBetweenVectors_UsingDeltaRotation(FVector InStart, FVector InTarget, FRotator InRotationToCheckAgainst);
	
	UFUNCTION(BlueprintGetter, Category = "Get Angle Between Vectors")
	static float GetAngleBetweenVectors_UsingArcTanDifference(FVector InStart, FVector InTarget, FVector InDirectionVector);


	
	//Sort Function
	UFUNCTION(BlueprintGetter = "Sort Array")
	TArray<float> SortArray_Float(const TArray<float> &ArrayToSort, bool bReversed) const;
	

	
	//Helper function - Set Array Element - Similar to Blueprint Node "Set Array Elem"
	template <typename ArrayType>
	static void SetArrayElement(TArray<ArrayType> &InArray, ArrayType Item, int32 Index, bool bSizeToFit)
	{
		if (bSizeToFit)
		{
			if (InArray.Num() - 1 < Index)
			{
				InArray.SetNum(Index);
				InArray.Insert(Item, Index);
			}
			else
			{
				InArray[Index] = Item;
			}
		}
		else if (!(InArray.Num() - 1 < Index))
		{
			InArray[Index] = Item;
		}
	}
	
	//Initialize variables
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = Initialization)
	void ServerInitializeVariables(UAIPerceptionComponent* InAIPerceptionComponent, AActor* InOwningCharacter);

	
	//Lock On functions (called from input)
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Lock-On")
	void ServerLockOnToTarget();

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Lock-On")
	void ServerLockOnToTarget_Left();

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Lock-On")
	void ServerLockOnToTarget_Right();
	
	

	
	//variables
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Replicated, Category = Initialization)
	bool bInitialize = true;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Actors|Owner")
	AActor* OwningActor;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = Components)
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Actors|Target")
	AActor* ActorToTarget;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Lock On Variables")
	bool bIsLockOnEnabled = false;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = TickGates)
	bool bOpenGateForTick = false;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Replicated, Category = "Perception Variables")
	bool AutoSwitchTargetsOnPerceptionFailure = true;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Replicated, Category = "Perception Variables")
	bool AutoSwitchTargetsOnActorToTargetInvalid = true;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Replicated, Category = "Lock on Variables")
	float InterpolationSpeed = 10;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Perception Variables", meta=(ToolTip = "This attempts to make sure that the current target is not part of the resulting sorted array"))
	float ErrorTolerance_Left = 5;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Perception Variables", meta=(ToolTip = "This attempts to make sure that the current target is not part of the resulting sorted array"))
	float ErrorTolerance_Right = 5;
	
	
protected:
	//overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//get perceived actors and store them in an array
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = StorePerceivedActors)
	void ServerStorePerceivedActorsInArray();

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = StorePerceivedActors)
	void ServerResetAllArrays();
	
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = FilterPerceivedActorsForTarget)
	void ServerDetermineActorWithTheLeastActorRotationAngleRequiredToTarget();

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = FilterPerceivedActorsForTarget)
	void ServerDetermineActorToTargetFromLowestRotationAngleArray();

	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = SwitchTargets)
	void ServerSwitchTargets(bool bSwitchTarget);


	//switch to target to the left of the current targeted actor
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = SwitchTargets)
	void ServerSwitchToTheTargetToTheLeftOfCurrentTarget();

	//switch to target to the right of the current targeted actor
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = SwitchTargets)
	void ServerSwitchToTheTargetToTheRightOfCurrentTarget();

	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = HideOrDisplayWidget)
	void ClientHideOrDisplayWidgetForLockedOnTarget(bool bHide, bool bUseDelay);


	
	//Prevent auto-camera rotation which is done on tick
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = TickGates)
	void ServerCloseGateForTick();

	UFUNCTION(NetMulticast, WithValidation, Reliable, BlueprintCallable, Category = TickGates)
	void MulticastCloseGateForTick();

	//Allow fixing the camera unto the target
	UFUNCTION(NetMulticast, WithValidation, Reliable, BlueprintCallable, Category = TickGates)
	void MulticastOpenGateForTick();

	//helper functions for Lock On
	UFUNCTION(Server, WithValidation, Reliable, BlueprintCallable, Category = "Lock-On")
	void ServerEnableOrDisableLockOn(bool bInEnable);

	

	//timers
	UFUNCTION(BlueprintCallable, Category = StorePerceivedActors)
	void Timer_StorePerceivedActorInArray();

	UFUNCTION(BlueprintCallable, Category = StorePerceivedActors)
	void ClearTimer_StorePerceivedActorsInArray();


	//Hide Widget For Target Not Being Locked On To, So Only The  Target Being Locked On To Displays The 3D Widget
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Lock On Variables|UI")
	void ClientShowWidgetForTargetNotBeingLockOnTo() const;

	//Hide Widget For Target Not Being Locked On To, So Only The  Target Being Locked On To Displays The 3D Widget
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Lock On Variables|UI")
	void ClientHideWidgetForTargetNotBeingLockOnTo() const;

	
	

	
	//function for tick
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void EnterLockOnClientGate();
	
	
	
	//replicated variables
	
	//store all perceived actors
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Actors|Perception", meta=(ToolTip = "Array to store all perceived actors"))
	TArray<AActor*> AllPerceivedActors;

	//should 3D widget in character be shown to indicate which character is locked-on to?
	UPROPERTY(BlueprintReadOnly, Replicated, EditDefaultsOnly, Category = "Lock On Variables|UI", meta=(ToolTip = "Should 3D widget in character be shown to indicate which character is locked-on to?"))
	bool ShouldShowWidgetForLockOn = true;
	
	//store each perceived actor's angle difference between the owning actor and the perceived actor
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "PerceptionVariables|AngleArrays", meta=(ToolTip = "Aray to store each perceived actor's angle difference between the owning actor and the perceived actor"))
	TArray<float> LowestRotationAngle_Array;

	
	//non-replicated variables
	UPROPERTY(BlueprintReadOnly, Category = "PerceptionVariables|AngleArrays|Left")
	TArray<float> LowestRotationAngle_Left_Array;

	UPROPERTY(BlueprintReadOnly, Category = "PerceptionVariables|AngleArrays|Left")
	TArray<float> LowestRotationAngle_Left_Array_Sorted;
	
	UPROPERTY(BlueprintReadOnly, Category = "PerceptionVariables|AngleArrays|Right")
	TArray<float> LowestRotationAngle_Right_Array;

	UPROPERTY(BlueprintReadOnly, Category = "PerceptionVariables|AngleArrays|Right")
	TArray<float> LowestRotationAngle_Right_Array_Sorted;

	
	//timers
	//delay between input and actual lock-on
	UPROPERTY(BlueprintReadOnly, Replicated, EditDefaultsOnly, Category = "Lock On Variables", meta=(ToolTip = "Delay between input and actual lock-on"))
	float DelayBeforeLockOn = 0.2;
	
	//How quickly should actors perceived be stored?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = PerceptionVariables, meta=(ToolTip = "How quickly should actors perceived be stored?"))
	float TimerIntervalForStoringPerceivedActorsInArray = 0.1;

	//Loop the timer or just store the perceived actors once?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = PerceptionVariables, meta=(ToolTip = "Loop the timer or just store the perceived actors once?"))
	bool bLoopTimerForStoringPerceivedActorsInArray = true;

	//timer handle for storing perceived actors in array - used to clear timer if timer should be stopped
	FTimerHandle TimerHandle_StorePerceivedActors;

	//timer handle for delay before lock-on
	FTimerHandle TimerHandle_DelayBeforeLockOn;


	
	//Sense To Use For Perception - Should be set to the dominant sense being used, or any preferred one
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = PerceptionVariables, meta=(ToolTip ="Sense To Use For Perception - Should be set to the dominant sense being used, or any preferred one"))
	TSubclassOf<UAISense> SenseToUseForPerception;

	//Should the Currently Perceived Actors Be Filtered Into A Particular Class?
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = PerceptionVariables, meta=(ToolTip = "Should the Currently Perceived Actors Be Filtered Into A Particular Class?"))
	bool bFilterPerceivedActorsArray = false;

	//Class to filter perceived actors into if bFilterPerceivedActorsArray is true
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = PerceptionVariables, meta=(ToolTip = "Class to filter perceived actors into if bFilterPerceivedActorsArray is true"))
	TSubclassOf<AActor> ClassToFilterPerceivedActorsTo = nullptr;

	

	//method to use to get angle difference between owning actor and perceived actor
	
	//If False, Control Rotation Is Used
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Perception Variables|Algorithm to Use for Getting Angle Between Vectors")
	bool bUseActorRotation = false;

	//Important For [Use Control Rotation]. If False, Arc Tan Difference Is Used
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Perception Variables|Algorithm to Use for Getting Angle Between Vectors")
	bool bUseDeltaRotation = true;
	
};
