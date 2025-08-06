// Copyright 2023 Ibrahim Akinde. Discord: Ragnar#9805. Email: ibmaxx@hotmail.com. Website: https://www.ibrahimakinde.com . All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TS_PlayerController.generated.h"

class UAIPerceptionComponent;
class UTargetingSystemComponent;

UCLASS()
class THETARGETINGSYSTEM_API ATS_PlayerController : public APlayerController
{
	GENERATED_BODY()
    
    //private
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Components, meta=(AllowPrivateAccess=true))
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Components,meta=(AllowPrivateAccess=true))
	UTargetingSystemComponent* TargetingSystemComponent;
	
	public:
	ATS_PlayerController();

    //call these to perform targeting
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void TargetCenter();

	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void TargetLeft();
	
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void TargetRight();

	//Getter Functions for private members
	UTargetingSystemComponent* GetTargetingSystemComponent() const {return TargetingSystemComponent;}
	UAIPerceptionComponent* GetPerceptionComponent() const {return AIPerceptionComponent;}

	protected:
	
	//configuration for Sight Sense for Perception
	UPROPERTY(BlueprintReadWrite, Category = Perception)
	class UAISenseConfig_Sight* SenseConfig_Sight;

    //overrides
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
};
