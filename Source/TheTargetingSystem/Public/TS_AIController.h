// Copyright 2023 Ibrahim Akinde. Discord: Ragnar#9805. Email: ibmaxx@hotmail.com. Website: https://www.ibrahimakinde.com . All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TS_AIController.generated.h"

class UTargetingSystemComponent;

UCLASS()
class THETARGETINGSYSTEM_API ATS_AIController : public AAIController
{
	GENERATED_BODY()
	
	//private
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Components, meta=(AllowPrivateAccess=true))
	UAIPerceptionComponent* AIPerceptionComponent;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = Components,meta=(AllowPrivateAccess=true))
	UTargetingSystemComponent* TargetingSystemComponent;
	
	public:

	ATS_AIController();
	
	//call these to perform targeting
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void TargetCenter() const;

	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void TargetLeft() const;
	
	UFUNCTION(BlueprintCallable, Category = "Lock-On")
	void TargetRight() const;

	//Getter Function for private member
	UTargetingSystemComponent* GetTargetingSystemComponent() const {return TargetingSystemComponent;}

	protected:
	
	//configuration for Sight Sense for Perception
	UPROPERTY(BlueprintReadWrite, Category = Perception)
	class UAISenseConfig_Sight* SenseConfig_Sight;

    //beginplay override
	virtual void BeginPlay() override;
	
};
