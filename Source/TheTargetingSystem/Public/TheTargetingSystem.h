// Copyright 2023 Ibrahim Akinde. Discord: Ragnar#9805. Email: ibmaxx@hotmail.com. Website: https://www.ibrahimakinde.com . All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FTheTargetingSystemModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
