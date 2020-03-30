// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "UObject/ObjectMacros.h"

class FToolBarBuilder;
class FMenuBuilder;
class SContentReference;
class SAssetDropTarget;
class FText;
class SFilterList;
class SAssetView;

UENUM()
enum class ColorChannel : uint8
{
	BLUE = 0,
	GREEN = 1,
	RED = 2,
	ALPHA = 3
};

class FWF_TexturePackerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void AddToolbarExtension( FToolBarBuilder& Builder );
	void AddMenuExtension( FMenuBuilder& Builder );

	TSharedRef<class SDockTab> OnSpawnPluginTab( const class FSpawnTabArgs& SpawnTabArgs );

	// SContentReference

	UObject* GetDisplaySourceTexture() const;
	UObject* GetRedSourceChannelTexture() const;
	UObject* GetGreenSourceChannelTexture() const;
	UObject* GetBlueSourceChannelTexture() const;
	UObject* GetAlphaSourceChannelTexture() const;

	UObject* GetRedTargetChannelTexture() const;
	UObject* GetGreenTargetChannelTexture() const;
	UObject* GetBlueTargetChannelTexture() const;
	UObject* GetAlphaTargetChannelTexture() const;

	FReply AssignAllSourceChannels();

	void SaveLocalCopyOfSourceTexture( UObject* texRef );
	void ExtractAllChannelsFromSourceTexture();

	void WriteChannelToSourceTexture( UObject* texRef, ColorChannel channelToWrite, ColorChannel channelToExtract );
	void WriteChannelToTargetTexture( UObject* texRef, ColorChannel channelToWrite, ColorChannel channelToExtract );

	FReply ClearAllChannels();

	void CreateErrorWindow(FText errorText);

	//SCreateAssetFromObject
	FReply SpawnAssetCreationWindow();
	void OnAssetCreation( const FString& inAssetPath );

	//SAssetDropTarget
	bool CheckAssetGrabbedCompatibleWithSourceTextureReference( const UObject* inObject ) const;
	bool CheckAssetGrabbedCompatibleWithNewChannels( const UObject* inObject ) const;

	void OnAssetDroppedOnSourceTexture( UObject* newAsset );
	void OnAssetDroppedOnRedChannelTexture( UObject* newAsset );
	void OnAssetDroppedOnGreenChannelTexture( UObject* newAsset );
	void OnAssetDroppedOnBlueChannelTexture( UObject* newAsset );
	void OnAssetDroppedOnAlphaChannelTexture( UObject* newAsset );
	void OnAssetDroppedOnChannelTexture( UObject* newAsset, ColorChannel channelDroppedOn );

	//SImage
	const FSlateBrush* GetSourceImageFromTexture() const;
	const FSlateBrush* GetSourceRedChannelImageFromTexture() const;
	const FSlateBrush* GetSourceGreenChannelImageFromTexture() const;
	const FSlateBrush* GetSourceBlueChannelImageFromTexture() const;
	const FSlateBrush* GetSourceAlphaChannelImageFromTexture() const;

	const FSlateBrush* GetTargetRedChannelImageFromTexture() const;
	const FSlateBrush* GetTargetGreenChannelImageFromTexture() const;
	const FSlateBrush* GetTargetBlueChannelImageFromTexture() const;
	const FSlateBrush* GetTargetAlphaChannelImageFromTexture() const;

	FReply OnSourceTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnSourceRedChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnSourceGreenChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnSourceBlueChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnSourceAlphaChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnSourceChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent, ColorChannel colorClicked );

	FReply OnTargetRedChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnTargetGreenChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnTargetBlueChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnTargetAlphaChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent );
	FReply OnTargetChannelTextureClicked( const FGeometry& myGeometry, const FPointerEvent& mouseEvent, ColorChannel colorClicked );

private:
	TSharedPtr< class FUICommandList > PluginCommands;

	TSharedPtr< class SContentReference > m_sourceTextureReference;

	TArray< TSharedPtr< class SContentReference > > m_sourceTextureChannelReferences;
	TArray< TSharedPtr< class SContentReference > > m_targetTextureChannelReferences;

	TSharedPtr< class SAssetDropTarget > m_sourceDropTarget;

	TArray< TSharedPtr< class SAssetDropTarget > > m_targetChannelsDropTargets;

	UObject* m_sourceTextureToDisplay;
	UTexture2D* m_sourceTexture;
	
	TArray< UObject* > m_sourceTextureChannels;
	TArray< UObject* > m_targetTextureChannels;
	TArray< uint8* > m_extractedSourceChannels;
	TArray< uint8* > m_writtenTargetChannels;

	bool m_hasExtractedSourceChannels;

	FSlateBrush m_sourceTextureSlateBrush;
	TArray< FSlateBrush > m_sourceTextureSlateBrushes;
	TArray< FSlateBrush > m_targetTextureSlateBrushes;

	const float m_universalDisplayTextureWidthSize = 140.0f;
	const float m_universalDisplayTextureHeightSize = 140.0f;

	int m_curTextureWidthSize;
	int m_curTextureHeightSize;
};
