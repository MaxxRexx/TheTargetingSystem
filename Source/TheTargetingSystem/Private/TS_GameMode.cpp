// Copyright 2023 Ibrahim Akinde. Discord: Ragnar#9805. Email: ibmaxx@hotmail.com. Website: https://www.ibrahimakinde.com . All Rights Reserved.


#include "TS_GameMode.h"

#include "TS_Character.h"
#include "TS_PlayerController.h"

ATS_GameMode::ATS_GameMode()
{
	DefaultPawnClass = ATS_Character::StaticClass();
	PlayerControllerClass = ATS_PlayerController::StaticClass();
}
