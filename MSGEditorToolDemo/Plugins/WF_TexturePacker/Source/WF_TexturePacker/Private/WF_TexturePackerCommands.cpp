// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "WF_TexturePackerCommands.h"

#define LOCTEXT_NAMESPACE "FWF_TexturePackerModule"

void FWF_TexturePackerCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "WF_TexturePacker", "Bring up WF_TexturePacker window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
