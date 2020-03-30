// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class MSGEditorToolDemoEditorTarget : TargetRules
{
	public MSGEditorToolDemoEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "MSGEditorToolDemo" } );
	}
}
