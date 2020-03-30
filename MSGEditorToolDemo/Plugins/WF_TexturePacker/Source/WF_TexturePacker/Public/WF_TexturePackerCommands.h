// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "WF_TexturePackerStyle.h"

class FWF_TexturePackerCommands : public TCommands<FWF_TexturePackerCommands>
{
public:

	FWF_TexturePackerCommands()
		: TCommands<FWF_TexturePackerCommands>(TEXT("WF_TexturePacker"), NSLOCTEXT("Contexts", "WF_TexturePacker", "WF_TexturePacker Plugin"), NAME_None, FWF_TexturePackerStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};