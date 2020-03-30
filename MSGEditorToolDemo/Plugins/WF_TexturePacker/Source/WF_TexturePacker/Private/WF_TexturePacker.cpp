// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "WF_TexturePacker.h"
#include "WF_TexturePackerStyle.h"
#include "WF_TexturePackerCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "WorkflowOrientedApp/SContentReference.h"
#include "Editor/PropertyEditor/Public/IDetailsView.h"
#include "Editor/UnrealEd/Public/SCreateAssetFromObject.h"
#include "Editor/KismetWidgets/Public/SSingleObjectDetailsPanel.h"
#include "Editor/EditorWidgets/Public/SAssetDropTarget.h"
#include "Runtime/Slate/Public/Widgets/Images/SImage.h"
#include "Editor/UnrealEd/Public/SEditorViewport.h"
#include "Editor.h"
#include "Editor/UnrealEd/Public/DragAndDrop/AssetDragDropOp.h"
//#include "Editor/PlacementMode/Public/PlacementModeModule.h"

static const FName WF_TexturePackerTabName("WF_TexturePacker");

#define LOCTEXT_NAMESPACE "FWF_TexturePackerModule"

const FString EnumToString(const TCHAR* Enum, int32 EnumValue)
{
	const UEnum* EnumPtr = FindObject< UEnum >(ANY_PACKAGE, Enum, true);
	if (!EnumPtr)
		return NSLOCTEXT("Invalid", "Invalid", "Invalid").ToString();

#if WITH_EDITOR
	return EnumPtr->GetDisplayNameTextByIndex(EnumValue).ToString();
#else
	return EnumPtr->GetEnumName(EnumValue);
#endif
}

const int redColorChannel = (int)ColorChannel::RED;
const int greenColorChannel = (int)ColorChannel::GREEN;
const int blueColorChannel = (int)ColorChannel::BLUE;
const int alphaColorChannel = (int)ColorChannel::ALPHA;

void FWF_TexturePackerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FWF_TexturePackerStyle::Initialize();
	FWF_TexturePackerStyle::ReloadTextures();

	FWF_TexturePackerCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FWF_TexturePackerCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FWF_TexturePackerModule::PluginButtonClicked),
		FCanExecuteAction());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked< FLevelEditorModule >("LevelEditor");

	{
		TSharedPtr< FExtender > MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FWF_TexturePackerModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}

	/*{
		TSharedPtr< FExtender > ToolbarExtender = MakeShareable( new FExtender );
		ToolbarExtender->AddToolBarExtension( "Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw( this, &FWF_TexturePackerModule::AddToolbarExtension ) );

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender( ToolbarExtender );
	}*/

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(WF_TexturePackerTabName, FOnSpawnTab::CreateRaw(this, &FWF_TexturePackerModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FWF_TexturePackerTabTitle", "WF_TexturePacker"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	m_sourceTextureChannels.Init(nullptr, 4);
	m_targetTextureChannels.Init(nullptr, 4);
	m_sourceTextureChannelReferences.Init(nullptr, 4);
	m_targetTextureChannelReferences.Init(nullptr, 4);
	m_extractedSourceChannels.Init(nullptr, 4);
	m_writtenTargetChannels.Init(nullptr, 4);
	m_targetChannelsDropTargets.Init(SNew(SAssetDropTarget), 4);
	m_sourceTextureSlateBrushes.Init(FSlateBrush(), 4);
	m_targetTextureSlateBrushes.Init(FSlateBrush(), 4);

	m_sourceTextureSlateBrush.DrawAs = ESlateBrushDrawType::NoDrawType;

	for (int i = 0; i < 4; ++i)
	{
		m_sourceTextureSlateBrushes[i].DrawAs = ESlateBrushDrawType::NoDrawType;
		m_targetTextureSlateBrushes[i].DrawAs = ESlateBrushDrawType::NoDrawType;
	}

	m_hasExtractedSourceChannels = false;
	m_curTextureWidthSize = 0;
	m_curTextureHeightSize = 0;
}

void FWF_TexturePackerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FWF_TexturePackerStyle::Shutdown();

	FWF_TexturePackerCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(WF_TexturePackerTabName);

	if (m_extractedSourceChannels[redColorChannel] != nullptr)
	{
		delete(m_extractedSourceChannels[redColorChannel]);
	}
	if (m_extractedSourceChannels[greenColorChannel] != nullptr)
	{
		delete(m_extractedSourceChannels[greenColorChannel]);
	}
	if (m_extractedSourceChannels[blueColorChannel] != nullptr)
	{
		delete(m_extractedSourceChannels[blueColorChannel]);
	}
	if (m_extractedSourceChannels[alphaColorChannel] != nullptr)
	{
		delete(m_extractedSourceChannels[alphaColorChannel]);
	}

	if (m_writtenTargetChannels[redColorChannel] != nullptr)
	{
		delete(m_writtenTargetChannels[redColorChannel]);
	}
	if (m_writtenTargetChannels[greenColorChannel] != nullptr)
	{
		delete(m_writtenTargetChannels[greenColorChannel]);
	}
	if (m_writtenTargetChannels[blueColorChannel] != nullptr)
	{
		delete(m_writtenTargetChannels[blueColorChannel]);
	}
	if (m_writtenTargetChannels[alphaColorChannel] != nullptr)
	{
		delete(m_writtenTargetChannels[alphaColorChannel]);
	}
}

void FWF_TexturePackerModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(WF_TexturePackerTabName);
}

void FWF_TexturePackerModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FWF_TexturePackerCommands::Get().OpenPluginWindow);
}

void FWF_TexturePackerModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FWF_TexturePackerCommands::Get().OpenPluginWindow);
}

TSharedRef< SDockTab > FWF_TexturePackerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText allButtonText = FText::FromString(TEXT("Extract all channels from Texture"));
	FText redButtonText = FText::FromString(TEXT("Extract Red channel from Texture"));
	FText greenButtonText = FText::FromString(TEXT("Extract Green channel from Texture"));
	FText blueButtonText = FText::FromString(TEXT("Extract Blue channel from Texture"));
	FText alphaButtonText = FText::FromString(TEXT("Extract Alpha channel from Texture"));
	FText createAssetText = FText::FromString(TEXT("Create Asset"));
	FText clearChannelsText = FText::FromString(TEXT("Clear Channels"));

	FText titleText = FText::FromString(TEXT("Title"));
	FText descriptionText = FText::FromString(TEXT("Short Description"));
	FText companyText = FText::FromString(TEXT("WayForward"));
	FText test = FText::FromString(TEXT("owo what's this?"));

	ClearAllChannels();

	TSharedRef< SDockTab > dockTabSharedRef = SNew(SDockTab);

	{
		SAssignNew(m_sourceTextureReference, SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetDisplaySourceTexture)
			.ShowFindInBrowserButton(true)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(true)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX)
			.OnSetReference_Raw(this, &FWF_TexturePackerModule::SaveLocalCopyOfSourceTexture);

		SAssignNew(m_sourceDropTarget, SAssetDropTarget)
			.OnIsAssetAcceptableForDrop_Raw(this, &FWF_TexturePackerModule::CheckAssetGrabbedCompatibleWithSourceTextureReference)
			.OnAssetDropped_Raw(this, &FWF_TexturePackerModule::OnAssetDroppedOnSourceTexture);

		TSharedRef< SImage > sourceTexImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetSourceImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnSourceTextureClicked);

		TSharedRef< SOverlay > sourceTexOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_sourceTextureReference.ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				sourceTexImage
			]
		+ SOverlay::Slot()
			[
				m_sourceDropTarget.ToSharedRef()
			];

		TSharedRef< SButton > clearChannelsButton = SNew(SButton)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Center)
			.Text(clearChannelsText)
			.ToolTipText(test)
			.OnClicked_Raw(this, &FWF_TexturePackerModule::ClearAllChannels);

		SAssignNew(m_sourceTextureChannelReferences[redColorChannel], SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetRedSourceChannelTexture)
			.ShowFindInBrowserButton(false)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX);

		TSharedRef< SImage > sourceTexRedChannelImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetSourceRedChannelImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnSourceRedChannelTextureClicked);

		TSharedRef< SOverlay > sourceTexRedChannelOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_sourceTextureChannelReferences[redColorChannel].ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				sourceTexRedChannelImage
			];

		SAssignNew(m_sourceTextureChannelReferences[greenColorChannel], SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetGreenSourceChannelTexture)
			.ShowFindInBrowserButton(false)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX);

		TSharedRef< SImage > sourceTexGreenChannelImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetSourceGreenChannelImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnSourceGreenChannelTextureClicked);

		TSharedRef< SOverlay > sourceTexGreenChannelOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_sourceTextureChannelReferences[greenColorChannel].ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				sourceTexGreenChannelImage
			];

		SAssignNew(m_sourceTextureChannelReferences[blueColorChannel], SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetBlueSourceChannelTexture)
			.ShowFindInBrowserButton(false)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX);

		TSharedRef< SImage > sourceTexBlueChannelImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetSourceBlueChannelImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnSourceBlueChannelTextureClicked);

		TSharedRef< SOverlay > sourceTexBlueChannelOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_sourceTextureChannelReferences[blueColorChannel].ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				sourceTexBlueChannelImage
			];

		SAssignNew(m_sourceTextureChannelReferences[alphaColorChannel], SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetAlphaSourceChannelTexture)
			.ShowFindInBrowserButton(false)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX);

		TSharedRef< SImage > sourceTexAlphaChannelImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetSourceAlphaChannelImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnSourceAlphaChannelTextureClicked);

		TSharedRef< SOverlay > sourceTexAlphaChannelOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_sourceTextureChannelReferences[alphaColorChannel].ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				sourceTexAlphaChannelImage
			];

		SAssignNew(m_targetTextureChannelReferences[redColorChannel], SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetRedTargetChannelTexture)
			.ShowFindInBrowserButton(false)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX);

		SAssignNew(m_targetChannelsDropTargets[redColorChannel], SAssetDropTarget)
			.OnIsAssetAcceptableForDrop_Raw(this, &FWF_TexturePackerModule::CheckAssetGrabbedCompatibleWithNewChannels)
			.OnAssetDropped_Raw(this, &FWF_TexturePackerModule::OnAssetDroppedOnRedChannelTexture);

		TSharedRef< SImage > targetTexRedChannelImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetTargetRedChannelImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnTargetRedChannelTextureClicked);

		TSharedRef< SOverlay > targetTexRedChannelOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_targetTextureChannelReferences[redColorChannel].ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				targetTexRedChannelImage
			]
		+ SOverlay::Slot()
			[
				m_targetChannelsDropTargets[redColorChannel].ToSharedRef()
			];

		SAssignNew(m_targetTextureChannelReferences[greenColorChannel], SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetGreenTargetChannelTexture)
			.ShowFindInBrowserButton(false)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX);

		SAssignNew(m_targetChannelsDropTargets[greenColorChannel], SAssetDropTarget)
			.OnIsAssetAcceptableForDrop_Raw(this, &FWF_TexturePackerModule::CheckAssetGrabbedCompatibleWithNewChannels)
			.OnAssetDropped_Raw(this, &FWF_TexturePackerModule::OnAssetDroppedOnGreenChannelTexture);

		TSharedRef< SImage > targetTexGreenChannelImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetTargetGreenChannelImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnTargetGreenChannelTextureClicked);

		TSharedRef< SOverlay > targetTexGreenChannelOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_targetTextureChannelReferences[greenColorChannel].ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				targetTexGreenChannelImage
			]
		+ SOverlay::Slot()
			[
				m_targetChannelsDropTargets[greenColorChannel].ToSharedRef()
			];

		SAssignNew(m_targetTextureChannelReferences[blueColorChannel], SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetBlueTargetChannelTexture)
			.ShowFindInBrowserButton(false)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX);

		SAssignNew(m_targetChannelsDropTargets[blueColorChannel], SAssetDropTarget)
			.OnIsAssetAcceptableForDrop_Raw(this, &FWF_TexturePackerModule::CheckAssetGrabbedCompatibleWithNewChannels)
			.OnAssetDropped_Raw(this, &FWF_TexturePackerModule::OnAssetDroppedOnBlueChannelTexture);

		TSharedRef< SImage > targetTexBlueChannelImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetTargetBlueChannelImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnTargetBlueChannelTextureClicked);

		TSharedRef< SOverlay > targetTexBlueChannelOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_targetTextureChannelReferences[blueColorChannel].ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				targetTexBlueChannelImage
			]
		+ SOverlay::Slot()
			[
				m_targetChannelsDropTargets[blueColorChannel].ToSharedRef()
			];

		SAssignNew(m_targetTextureChannelReferences[alphaColorChannel], SContentReference)
			.Style(TEXT("ContentReference"))
			.AssetReference_Raw(this, &FWF_TexturePackerModule::GetAlphaTargetChannelTexture)
			.ShowFindInBrowserButton(false)
			.ShowToolsButton(false)
			.AllowSelectingNewAsset(false)
			.AllowClearingReference(false)
			.AllowedClass(UTexture2D::StaticClass())
			.WidthOverride(FOptionalSize(m_universalDisplayTextureWidthSize - 10.0f))
			.InitialAssetViewType(EAssetViewType::MAX);

		SAssignNew(m_targetChannelsDropTargets[alphaColorChannel], SAssetDropTarget)
			.OnIsAssetAcceptableForDrop_Raw(this, &FWF_TexturePackerModule::CheckAssetGrabbedCompatibleWithNewChannels)
			.OnAssetDropped_Raw(this, &FWF_TexturePackerModule::OnAssetDroppedOnAlphaChannelTexture);

		TSharedRef< SImage > targetTexAlphaChannelImage = SNew(SImage)
			.Image_Raw(this, &FWF_TexturePackerModule::GetTargetAlphaChannelImageFromTexture)
			.OnMouseButtonDown_Raw(this, &FWF_TexturePackerModule::OnTargetAlphaChannelTextureClicked);

		TSharedRef< SOverlay > targetTexAlphaChannelOverlay = SNew(SOverlay)
			+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				m_targetTextureChannelReferences[alphaColorChannel].ToSharedRef()
			]
		+ SOverlay::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				targetTexAlphaChannelImage
			]
		+ SOverlay::Slot()
			[
				m_targetChannelsDropTargets[alphaColorChannel].ToSharedRef()
			];

		TSharedRef< SButton > assetCreatorButton = SNew(SButton)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.Text(createAssetText)
			.ToolTipText(test)
			.OnClicked_Raw(this, &FWF_TexturePackerModule::SpawnAssetCreationWindow);

		const TSharedRef< FUICommandList > CommandList = MakeShareable(new FUICommandList);
		//const TSharedRef< FTabManager > tabManager = MakeShareable( new FTabManager( dockTabSharedRef, );

		TSharedRef< SVerticalBox > verticalBox = SNew(SVerticalBox);
		TSharedRef< SHorizontalBox > horizontalBoxTop = SNew(SHorizontalBox);
		TSharedRef< SHorizontalBox > horizontalBoxMid = SNew(SHorizontalBox);
		TSharedRef< SHorizontalBox > horizontalBoxBot = SNew(SHorizontalBox);
		TSharedRef< SHorizontalBox > horizontalBoxFarBot = SNew(SHorizontalBox);

		horizontalBoxTop->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				sourceTexOverlay
			];

		horizontalBoxTop->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Right)
			.Padding(30.0f)
			[
				clearChannelsButton
			];

		horizontalBoxMid->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				sourceTexRedChannelOverlay
			];
		horizontalBoxMid->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				sourceTexGreenChannelOverlay
			];
		horizontalBoxMid->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				sourceTexBlueChannelOverlay
			];
		horizontalBoxMid->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				sourceTexAlphaChannelOverlay
			];

		horizontalBoxBot->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				targetTexRedChannelOverlay
			];
		horizontalBoxBot->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				targetTexGreenChannelOverlay
			];
		horizontalBoxBot->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				targetTexBlueChannelOverlay
			];
		horizontalBoxBot->AddSlot()
			.MaxWidth(m_universalDisplayTextureWidthSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.Padding(10.0f)
			[
				targetTexAlphaChannelOverlay
			];

		horizontalBoxFarBot->AddSlot()
			.FillWidth(1.0f)
			[
				assetCreatorButton
			];

		verticalBox->AddSlot()
			.MaxHeight(m_universalDisplayTextureHeightSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				horizontalBoxTop
			];
		verticalBox->AddSlot()
			.MaxHeight(m_universalDisplayTextureHeightSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				horizontalBoxMid
			];
		verticalBox->AddSlot()
			.MaxHeight(m_universalDisplayTextureHeightSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				horizontalBoxBot
			];
		verticalBox->AddSlot()
			.MaxHeight(m_universalDisplayTextureHeightSize)
			.VAlign(EVerticalAlignment::VAlign_Fill)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.Padding(20.0f)
			[
				horizontalBoxFarBot
			];

		dockTabSharedRef->SetContent(verticalBox);
	}

	//SpawnTabArgs.GetOwnerWindow().Get().

	return dockTabSharedRef;
}

UObject* FWF_TexturePackerModule::GetDisplaySourceTexture() const
{
	return m_sourceTextureToDisplay;
}

UObject* FWF_TexturePackerModule::GetRedSourceChannelTexture() const
{
	return m_sourceTextureChannels[redColorChannel];
}

UObject* FWF_TexturePackerModule::GetGreenSourceChannelTexture() const
{
	return m_sourceTextureChannels[greenColorChannel];
}

UObject* FWF_TexturePackerModule::GetBlueSourceChannelTexture() const
{
	return m_sourceTextureChannels[blueColorChannel];
}

UObject* FWF_TexturePackerModule::GetAlphaSourceChannelTexture() const
{
	return m_sourceTextureChannels[alphaColorChannel];
}

UObject* FWF_TexturePackerModule::GetRedTargetChannelTexture() const
{
	return m_targetTextureChannels[redColorChannel];
}

UObject* FWF_TexturePackerModule::GetGreenTargetChannelTexture() const
{
	return m_targetTextureChannels[greenColorChannel];
}

UObject* FWF_TexturePackerModule::GetBlueTargetChannelTexture() const
{
	return m_targetTextureChannels[blueColorChannel];
}

UObject* FWF_TexturePackerModule::GetAlphaTargetChannelTexture() const
{
	return m_targetTextureChannels[alphaColorChannel];
}

FReply FWF_TexturePackerModule::AssignAllSourceChannels()
{
	if (m_sourceTexture != nullptr)
	{
		if (false == m_hasExtractedSourceChannels)
		{
			ExtractAllChannelsFromSourceTexture();
		}

		WriteChannelToSourceTexture(m_sourceTextureToDisplay, ColorChannel::RED, ColorChannel::RED);
		WriteChannelToSourceTexture(m_sourceTextureToDisplay, ColorChannel::GREEN, ColorChannel::GREEN);
		WriteChannelToSourceTexture(m_sourceTextureToDisplay, ColorChannel::BLUE, ColorChannel::BLUE);
		WriteChannelToSourceTexture(m_sourceTextureToDisplay, ColorChannel::ALPHA, ColorChannel::ALPHA);

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void FWF_TexturePackerModule::SaveLocalCopyOfSourceTexture(UObject* texRef)
{
	if (texRef)
	{
		UTexture2D* tex = Cast< UTexture2D >(texRef);

		if (tex)
		{
			if (TextureCompressionSettings::TC_VectorDisplacementmap == tex->CompressionSettings
				|| TextureCompressionSettings::TC_EditorIcon == tex->CompressionSettings)
			{
				tex->UpdateResource();
				FTexture2DMipMap& readMip = tex->PlatformData->Mips[0];
				//UE_LOG( LogTemp, Warning, TEXT( "mipmap size: %d" ), readMip.BulkData.GetBulkDataSize() );

				void* texData = readMip.BulkData.Lock(LOCK_READ_WRITE);

				if (texData != nullptr)
				{
					readMip.BulkData.Unlock();

					if ((m_curTextureWidthSize == 0 && m_curTextureHeightSize == 0)
						|| (m_curTextureWidthSize == tex->GetSizeX() && m_curTextureHeightSize == tex->GetSizeY()))
					{

						m_sourceTextureToDisplay = texRef;
						m_curTextureWidthSize = tex->GetSizeX();
						m_curTextureHeightSize = tex->GetSizeY();
						//UE_LOG( LogTemp, Warning, TEXT( "%s assigned!" ), *texRef->GetName() );

						/*if ( m_sourceTexture != nullptr )
						{
							m_sourceTexture->ConditionalBeginDestroy();
						}*/

						m_sourceTexture = tex;

						m_sourceTextureSlateBrush.SetResourceObject(m_sourceTexture);
						m_sourceTextureSlateBrush.ImageSize.X = m_universalDisplayTextureWidthSize;
						m_sourceTextureSlateBrush.ImageSize.Y = m_sourceTexture->GetSizeY();
						m_sourceTextureSlateBrush.DrawAs = ESlateBrushDrawType::Image;

						m_hasExtractedSourceChannels = false;

						ExtractAllChannelsFromSourceTexture();
						AssignAllSourceChannels();
					}
					else
					{
						CreateErrorWindow(LOCTEXT("InvalidTextureSize", "ERROR: This image has different dimensions than the current channels.  This tool does not support\ncombining channels of different sizes.  Please clear channels if you want to restart texture channel extraction."));
					}
				}
				else
				{
					CreateErrorWindow(LOCTEXT("InvalidMipGeneration", "ERROR: This image has not correctly generated it's Mipmaps.  Please go to the \"Level Of Detail\" category\nin the texture settings and set \"Mip Gen Settings\" to \"NoMipMaps\"."));

					readMip.BulkData.Unlock();
					return;
				}
			}
			else
			{
				CreateErrorWindow(LOCTEXT("InvalidTextureCompression", "ERROR: This tool only supports uncompressed textures.  Please make sure your textures compression\nsettings are set to \"VectorDisplacementMap(RGBA8)\" or \"UserInterface2D(RGBA)\"."));
			}
		}
	}
}

void FWF_TexturePackerModule::ExtractAllChannelsFromSourceTexture()
{
	if (m_sourceTexture != nullptr)
	{
		// Update the variables since we have already uncompressed it when we saved it
		//m_sourceTexture->UpdateResource();

		int32 w, h;
		w = m_sourceTexture->GetSizeX();
		h = m_sourceTexture->GetSizeY();

		// * 4 because each pixel has 4 uint8's, which are each 1 byte large
		int dataSizeInBytes = w * h * 4;

		int dataSizeInPixels = w * h;

		// Initalize our dynamic pixel array with data size
		uint8* dynamicColors = new uint8[dataSizeInBytes];

		FTexture2DMipMap& readMip = m_sourceTexture->PlatformData->Mips[0];
		//UE_LOG( LogTemp, Warning, TEXT( "mipmap size: %d" ), readMip.BulkData.GetBulkDataSize() );

		void* texData = readMip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(dynamicColors, texData, dataSizeInBytes);

		if (m_extractedSourceChannels[redColorChannel] != nullptr)
		{
			delete(m_extractedSourceChannels[redColorChannel]);
		}
		if (m_extractedSourceChannels[greenColorChannel] != nullptr)
		{
			delete(m_extractedSourceChannels[greenColorChannel]);
		}
		if (m_extractedSourceChannels[blueColorChannel] != nullptr)
		{
			delete(m_extractedSourceChannels[blueColorChannel]);
		}
		if (m_extractedSourceChannels[alphaColorChannel] != nullptr)
		{
			delete(m_extractedSourceChannels[alphaColorChannel]);
		}

		// Initialize our pixels for each specific channel
		m_extractedSourceChannels[redColorChannel] = new uint8[dataSizeInBytes];
		m_extractedSourceChannels[greenColorChannel] = new uint8[dataSizeInBytes];
		m_extractedSourceChannels[blueColorChannel] = new uint8[dataSizeInBytes];
		m_extractedSourceChannels[alphaColorChannel] = new uint8[dataSizeInBytes];

		//UE_LOG( LogTemp, Warning, TEXT( "bytes allocated per texture: %d" ), dataSizeInBytes );

		// Strip every other channel out for textures to only contain it's intended channel
		int blue, green, red, alpha;

		for (int i = 0; i < dataSizeInPixels; ++i)
		{
			blue = (i * 4) + 0;
			green = (i * 4) + 1;
			red = (i * 4) + 2;
			alpha = (i * 4) + 3;

			// Set pixel's red value to 120 
			//dynamicColors[red] = 120;

			m_extractedSourceChannels[blueColorChannel][blue] = dynamicColors[blue];
			m_extractedSourceChannels[blueColorChannel][green] = (uint8)0;
			m_extractedSourceChannels[blueColorChannel][red] = (uint8)0;
			m_extractedSourceChannels[blueColorChannel][alpha] = 255;// dynamicColors[alpha];

			m_extractedSourceChannels[greenColorChannel][blue] = (uint8)0;
			m_extractedSourceChannels[greenColorChannel][green] = dynamicColors[green];
			m_extractedSourceChannels[greenColorChannel][red] = (uint8)0;
			m_extractedSourceChannels[greenColorChannel][alpha] = 255;// dynamicColors[alpha];

			m_extractedSourceChannels[redColorChannel][blue] = (uint8)0;
			m_extractedSourceChannels[redColorChannel][green] = (uint8)0;
			m_extractedSourceChannels[redColorChannel][red] = dynamicColors[red];
			m_extractedSourceChannels[redColorChannel][alpha] = 255;// dynamicColors[alpha];

			m_extractedSourceChannels[alphaColorChannel][blue] = (uint8)0;
			m_extractedSourceChannels[alphaColorChannel][green] = (uint8)0;
			m_extractedSourceChannels[alphaColorChannel][red] = (uint8)0;
			m_extractedSourceChannels[alphaColorChannel][alpha] = dynamicColors[alpha];
		}

		readMip.BulkData.Unlock();

		if (dynamicColors != nullptr)
		{
			delete(dynamicColors);
		}

		m_hasExtractedSourceChannels = true;
	}
}

void FWF_TexturePackerModule::WriteChannelToSourceTexture(UObject* texRef, ColorChannel channelToWrite, ColorChannel channelToExtract)
{
	if (texRef)
	{
		UTexture2D* tex = Cast< UTexture2D >(texRef);

		if (tex)
		{
			FString texString = "sourceTex";

			int channelExtractingFrom = (int)channelToExtract;
			int channelWritingTo = (int)channelToWrite;

			if (m_sourceTextureChannels[channelWritingTo] != nullptr)
			{
				m_sourceTextureChannels[channelWritingTo]->ConditionalBeginDestroy();
			}

			texString += EnumToString(TEXT("ColorChannel"), static_cast<uint8>(channelToWrite));

			FName texName = FName(*texString);

			UTexture2D* dynamicTex = NewObject< UTexture2D >((UObject*)GetTransientPackage(), UTexture2D::StaticClass(), texName, RF_NoFlags, Cast< UObject >(m_sourceTexture));

			//Guarantee no garbage collection by adding it as a root reference
			dynamicTex->AddToRoot();

			//Update the texture with new variable values.
			dynamicTex->UpdateResource();


			// * 4 because each pixel has 4 uint8's, which are each 1 byte large
			int dataSizeInBytes = m_sourceTexture->GetSizeX() * m_sourceTexture->GetSizeY() * 4;

			FTexture2DMipMap& mip = dynamicTex->PlatformData->Mips[0];
			//UE_LOG( LogTemp, Warning, TEXT( "dynamicTex mipmap size: %d" ), mip.BulkData.GetBulkDataSize() );
			void* data = mip.BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(data, m_extractedSourceChannels[channelExtractingFrom], dataSizeInBytes);
			mip.BulkData.Unlock();
			dynamicTex->UpdateResource();

			UObject* texToSave = Cast< UObject >(dynamicTex);

			if (texToSave)
			{
				m_sourceTextureChannels[channelWritingTo] = texToSave;

				m_sourceTextureSlateBrushes[channelWritingTo].SetResourceObject(dynamicTex);
				m_sourceTextureSlateBrushes[channelWritingTo].ImageSize.X = m_universalDisplayTextureWidthSize;
				m_sourceTextureSlateBrushes[channelWritingTo].ImageSize.Y = dynamicTex->GetSizeY();
				m_sourceTextureSlateBrushes[channelWritingTo].DrawAs = ESlateBrushDrawType::Image;
			}
		}
	}
}

void FWF_TexturePackerModule::WriteChannelToTargetTexture(UObject* texRef, ColorChannel channelToWrite, ColorChannel channelToExtract)
{
	if (texRef)
	{
		UTexture2D* tex = Cast< UTexture2D >(texRef);

		if (tex)
		{
			FString texString = "newTex";
			uint8* channelToUse = nullptr;

			if (texRef == m_sourceTextureChannels[redColorChannel])
			{
				channelToExtract = ColorChannel::RED;
			}
			else if (texRef == m_sourceTextureChannels[greenColorChannel])
			{
				channelToExtract = ColorChannel::GREEN;
			}
			else if (texRef == m_sourceTextureChannels[blueColorChannel])
			{
				channelToExtract = ColorChannel::BLUE;
			}
			else if (texRef == m_sourceTextureChannels[alphaColorChannel])
			{
				channelToExtract = ColorChannel::ALPHA;
			}

			int channelExtractingFrom = (int)channelToExtract;
			int channelWritingTo = (int)channelToWrite;

			if (m_targetTextureChannels[channelWritingTo] != nullptr)
			{
				m_targetTextureChannels[channelWritingTo]->ConditionalBeginDestroy();
			}

			texString += EnumToString(TEXT("ColorChannel"), static_cast<uint8>(channelToWrite));

			FName texName = FName(*texString);

			UTexture2D* dynamicTex = NewObject< UTexture2D >((UObject*)GetTransientPackage(), UTexture2D::StaticClass(), texName, RF_NoFlags, texRef);

			//Guarantee no garbage collection by adding it as a root reference
			dynamicTex->AddToRoot();

			//Update the texture with new variable values.
			dynamicTex->UpdateResource();

			// * 4 because each pixel has 4 uint8's, which are each 1 byte large
			int dataSizeInBytes = m_sourceTexture->GetSizeX() * m_sourceTexture->GetSizeY() * 4;
			uint8* newPixelData = new uint8[dataSizeInBytes];

			int dataSizeInPixels = dataSizeInBytes / 4;

			int blue, green, red, alpha;

			for (int i = 0; i < dataSizeInPixels; ++i)
			{
				blue = (i * 4) + 0;
				green = (i * 4) + 1;
				red = (i * 4) + 2;
				alpha = (i * 4) + 3;

				newPixelData[blue] = 0;
				newPixelData[green] = 0;
				newPixelData[red] = 0;
				newPixelData[alpha] = 255;//m_extractedSourceChannels[channelExtractingFrom][alpha];

				newPixelData[(i * 4) + channelWritingTo] = m_extractedSourceChannels[channelExtractingFrom][(i * 4) + channelExtractingFrom];
			}

			m_writtenTargetChannels[channelWritingTo] = newPixelData;

			FTexture2DMipMap& mip = dynamicTex->PlatformData->Mips[0];
			//UE_LOG( LogTemp, Warning, TEXT( "dynamicTex mipmap size: %d" ), mip.BulkData.GetBulkDataSize() );
			void* data = mip.BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(data, newPixelData, dataSizeInBytes);
			mip.BulkData.Unlock();
			dynamicTex->UpdateResource();

			UObject* texToSave = Cast< UObject >(dynamicTex);

			if (texToSave)
			{
				m_targetTextureChannels[channelWritingTo] = texToSave;

				m_targetTextureSlateBrushes[channelWritingTo].SetResourceObject(dynamicTex);
				m_targetTextureSlateBrushes[channelWritingTo].ImageSize.X = m_universalDisplayTextureWidthSize;
				m_targetTextureSlateBrushes[channelWritingTo].ImageSize.Y = dynamicTex->GetSizeY();
				m_targetTextureSlateBrushes[channelWritingTo].DrawAs = ESlateBrushDrawType::Image;
			}
		}
	}
}

FReply FWF_TexturePackerModule::ClearAllChannels()
{
	m_curTextureWidthSize = 0;
	m_curTextureHeightSize = 0;
	if (m_sourceTexture != nullptr)
	{
		m_sourceTexture = nullptr;
		m_sourceTextureSlateBrush.SetResourceObject(nullptr);
		m_sourceTextureSlateBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
		m_sourceTextureToDisplay = nullptr;
	}

	for (int i = 0; i < 4; ++i)
	{
		if (m_sourceTextureChannels[i] != nullptr)
		{
			m_sourceTextureChannels[i]->ConditionalBeginDestroy();
		}

		if (m_targetTextureChannels[i] != nullptr)
		{
			m_targetTextureChannels[i]->ConditionalBeginDestroy();
		}
	}

	return FReply::Handled();
}

void FWF_TexturePackerModule::CreateErrorWindow(FText errorText)
{
	TSharedPtr< SWindow > errorWindow;
	SAssignNew(errorWindow, SWindow)
		.Title(LOCTEXT("Error", "Error"))
		.ClientSize(FVector2D(650, 100));

	TSharedPtr< STextBlock > textBox;
	errorWindow->SetContent
	(
		SNew(SBox)
		.Padding(20.0f)
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
			SAssignNew(textBox, STextBlock)
			.Text(errorText)
		]
	);

	TSharedPtr< SWindow > rootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (rootWindow.IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(errorWindow.ToSharedRef(), rootWindow.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(errorWindow.ToSharedRef());
	}
}

FReply FWF_TexturePackerModule::SpawnAssetCreationWindow()
{
	TSharedPtr< SWindow > pickPath;
	SAssignNew(pickPath, SWindow)
		.Title(LOCTEXT("SelectPath", "Select Path"))
		.ToolTipText(LOCTEXT("SelectPathTooltip", "Select the path where the Blueprint will be created at"))
		.ClientSize(FVector2D(400, 400));

	TSharedPtr< SCreateAssetFromObject > createAsset;
	pickPath->SetContent
	(
		SAssignNew(createAsset, SCreateAssetFromObject, pickPath)
		.AssetFilenameSuffix(TEXT("NewUncompressedTexture"))
		.HeadingText(LOCTEXT("CreateAsset_Heading", "Asset Name"))
		.CreateButtonText(LOCTEXT("CreateAsset_ButtonLabel", "Create Asset"))
		.OnCreateAssetAction(FOnPathChosen::CreateRaw(this, &FWF_TexturePackerModule::OnAssetCreation))
	);

	TSharedPtr< SWindow > RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
	if (RootWindow.IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(pickPath.ToSharedRef(), RootWindow.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(pickPath.ToSharedRef());
	}

	return FReply::Handled();
}

void FWF_TexturePackerModule::OnAssetCreation(const FString& inAssetPath)
{
	UPackage* package = CreatePackage(nullptr, *inAssetPath);
	int32 stringIndex = inAssetPath.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd, inAssetPath.Len());
	FString newTextureName = inAssetPath.Right((inAssetPath.Len() - stringIndex) - 1);

	UTexture2D* newTexture = NewObject< UTexture2D >(package, UTexture2D::StaticClass(), *newTextureName, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone);
	newTexture->UpdateResource();

	int widthSize = m_sourceTexture->GetSizeX();
	int heightSize = m_sourceTexture->GetSizeY();

	// * 4 because each pixel has 4 uint8's, which are each 1 byte large
	int dataSizeInBytes = widthSize * heightSize * 4;
	uint8* newPixelData = new uint8[dataSizeInBytes];

	int dataSizeInPixels = dataSizeInBytes / 4;

	int blue, green, red, alpha;

	for (int i = 0; i < dataSizeInPixels; ++i)
	{
		blue = (i * 4) + 0;
		green = (i * 4) + 1;
		red = (i * 4) + 2;
		alpha = (i * 4) + 3;

		newPixelData[blue] = m_writtenTargetChannels[blueColorChannel][blue];
		newPixelData[green] = m_writtenTargetChannels[greenColorChannel][green];
		newPixelData[red] = m_writtenTargetChannels[redColorChannel][red];
		newPixelData[alpha] = m_writtenTargetChannels[alphaColorChannel][alpha];
	}

	/**
	// Save a copy of all MIP data for later color lookups
	// Doing GetMipData for each sample would allocate and copy the entire MIP chain for each access
	TArray< FColor* > mipData;
	int32 numSourceMips = m_sourceTexture->GetNumMips();
	int32 firstMipIndex = 0;
	mipData.AddZeroed( numSourceMips );
	m_sourceTexture->GetMipData( firstMipIndex, ( void** )mipData.GetData() );

	// Copy just the needed MIP data and adjust size
	// Don't ask me about the math going on here, I copied it from "Runtime\Engine\Private\Kismet\ImportanceSamplingLibrary.cpp" LINE 95 - SB
	FIntPoint srcSize = FIntPoint( widthSize, heightSize );
	FIntPoint newSize = FIntPoint( ( ( srcSize.X - 1 ) > > firstMipIndex ) + 1, ( ( srcSize.Y - 1 ) > > firstMipIndex ) + 1 );
	FIntPoint lastMipSize( ( ( newSize.X - 1 ) > > ( numSourceMips - 1 ) ) + 1, ( ( newSize.Y - 1 ) > > ( numSourceMips - 1 ) ) + 1 );
	size_t mipDataSize = ( 4 * newSize.X * newSize.Y - lastMipSize.X * lastMipSize.Y ) / 3;
	TArray< FColor > textureData;
	textureData.SetNumUninitialized( mipDataSize );
	for ( int32 mip = 0; mip < numSourceMips; ++mip )
	{
		FIntPoint levelSize( ( ( newSize.X - 1 ) > > mip ) + 1, ( ( newSize.Y - 1 ) > > mip ) + 1 );
		size_t levelStart = ( newSize.X * newSize.Y - levelSize.X * levelSize.Y ) * 4 / 3;
		FMemory::Memcpy( ( void* )&textureData[levelStart], ( void* )mipData[mip], levelSize.X*levelSize.Y * sizeof( FColor ) );
	}

	// get rid of temporary copy of MIP data
	for ( auto mipLevel : mipData )
	{
		FMemory::Free( mipLevel );
	}
	*/

	newTexture->PlatformData->SizeX = widthSize;
	newTexture->PlatformData->SizeY = heightSize;
	newTexture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;
	newTexture->Filter = m_sourceTexture->Filter;
	newTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	newTexture->SRGB = m_sourceTexture->SRGB;
	newTexture->bUseLegacyGamma = m_sourceTexture->bUseLegacyGamma;


	FTexture2DMipMap* mipMap = new(newTexture->PlatformData->Mips) FTexture2DMipMap();
	mipMap->SizeX = widthSize;
	mipMap->SizeY = heightSize;

	// Lock the texture so it can be modified
	mipMap->BulkData.Lock(LOCK_READ_WRITE);
	mipMap->BulkData.Realloc(dataSizeInBytes);
	mipMap->BulkData.Unlock();

	// Relock to write data
	void* newMipData = mipMap->BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(newMipData, newPixelData, dataSizeInBytes);
	mipMap->BulkData.Unlock();

	// We call this to make sure the PlatformData is saved by the Packager
	newTexture->Source.Init(widthSize, heightSize, 1, 1, ETextureSourceFormat::TSF_BGRA8, newPixelData);

	newTexture->UpdateResource();

	if (newPixelData != nullptr)
	{
		delete(newPixelData);
	}

	bool successfullySaved = UPackage::SavePackage(package, Cast< UObject >(newTexture), EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *newTextureName);

	UE_LOG(LogTemp, Warning, TEXT("Saved Package: %s to %s"), successfullySaved ? TEXT("True") : TEXT("False"), *package->GetPathName());
}

bool FWF_TexturePackerModule::CheckAssetGrabbedCompatibleWithSourceTextureReference(const UObject* inObject) const
{
	if (m_sourceTextureChannels[redColorChannel] != inObject
		&& m_sourceTextureChannels[greenColorChannel] != inObject
		&& m_sourceTextureChannels[blueColorChannel] != inObject
		&& m_sourceTextureChannels[alphaColorChannel] != inObject)
	{
		if (Cast< UTexture2D >(inObject) != nullptr)
		{
			m_sourceDropTarget->SetVisibility(EVisibility::Visible);
			return true;
		}
	}

	m_sourceDropTarget->SetVisibility(EVisibility::SelfHitTestInvisible);
	return false;
}

bool FWF_TexturePackerModule::CheckAssetGrabbedCompatibleWithNewChannels(const UObject* inObject) const
{
	if (m_sourceTextureChannels[redColorChannel] == inObject
		|| m_sourceTextureChannels[greenColorChannel] == inObject
		|| m_sourceTextureChannels[blueColorChannel] == inObject
		|| m_sourceTextureChannels[alphaColorChannel] == inObject)
	{
		for (int i = 0; i < 4; ++i)
		{
			m_targetChannelsDropTargets[i]->SetVisibility(EVisibility::Visible);
		}
		return true;
	}

	for (int i = 0; i < 4; ++i)
	{
		m_targetChannelsDropTargets[i]->SetVisibility(EVisibility::SelfHitTestInvisible);
	}

	return false;

}

void FWF_TexturePackerModule::OnAssetDroppedOnSourceTexture(UObject* newAsset)
{
	m_sourceDropTarget->SetVisibility(EVisibility::SelfHitTestInvisible);
	SaveLocalCopyOfSourceTexture(newAsset);
}

void FWF_TexturePackerModule::OnAssetDroppedOnRedChannelTexture(UObject* newAsset)
{
	OnAssetDroppedOnChannelTexture(newAsset, ColorChannel::RED);
}

void FWF_TexturePackerModule::OnAssetDroppedOnGreenChannelTexture(UObject* newAsset)
{
	OnAssetDroppedOnChannelTexture(newAsset, ColorChannel::GREEN);
}

void FWF_TexturePackerModule::OnAssetDroppedOnBlueChannelTexture(UObject* newAsset)
{
	OnAssetDroppedOnChannelTexture(newAsset, ColorChannel::BLUE);
}

void FWF_TexturePackerModule::OnAssetDroppedOnAlphaChannelTexture(UObject* newAsset)
{
	OnAssetDroppedOnChannelTexture(newAsset, ColorChannel::ALPHA);
}

void FWF_TexturePackerModule::OnAssetDroppedOnChannelTexture(UObject* newAsset, ColorChannel channelDroppedOn)
{
	for (int i = 0; i < 4; ++i)
	{
		m_targetChannelsDropTargets[i]->SetVisibility(EVisibility::SelfHitTestInvisible);
	}

	ColorChannel channelToExtract;

	if (newAsset == m_sourceTextureChannels[redColorChannel])
	{
		channelToExtract = ColorChannel::RED;
	}
	else if (newAsset == m_sourceTextureChannels[greenColorChannel])
	{
		channelToExtract = ColorChannel::GREEN;
	}
	else if (newAsset == m_sourceTextureChannels[blueColorChannel])
	{
		channelToExtract = ColorChannel::BLUE;
	}
	else if (newAsset == m_sourceTextureChannels[alphaColorChannel])
	{
		channelToExtract = ColorChannel::ALPHA;
	}
	else
	{
		// Only allow dropping source channels onto target channels.
		return;
	}

	WriteChannelToTargetTexture(newAsset, channelDroppedOn, channelToExtract);
}

const FSlateBrush* FWF_TexturePackerModule::GetSourceImageFromTexture() const
{
	return &m_sourceTextureSlateBrush;
}

const FSlateBrush* FWF_TexturePackerModule::GetSourceRedChannelImageFromTexture() const
{
	return &m_sourceTextureSlateBrushes[redColorChannel];
}

const FSlateBrush* FWF_TexturePackerModule::GetSourceGreenChannelImageFromTexture() const
{
	return &m_sourceTextureSlateBrushes[greenColorChannel];
}

const FSlateBrush* FWF_TexturePackerModule::GetSourceBlueChannelImageFromTexture() const
{
	return &m_sourceTextureSlateBrushes[blueColorChannel];
}

const FSlateBrush* FWF_TexturePackerModule::GetSourceAlphaChannelImageFromTexture() const
{
	return &m_sourceTextureSlateBrushes[alphaColorChannel];
}

const FSlateBrush* FWF_TexturePackerModule::GetTargetRedChannelImageFromTexture() const
{
	return &m_targetTextureSlateBrushes[redColorChannel];
}

const FSlateBrush* FWF_TexturePackerModule::GetTargetGreenChannelImageFromTexture() const
{
	return &m_targetTextureSlateBrushes[greenColorChannel];
}

const FSlateBrush* FWF_TexturePackerModule::GetTargetBlueChannelImageFromTexture() const
{
	return &m_targetTextureSlateBrushes[blueColorChannel];
}

const FSlateBrush* FWF_TexturePackerModule::GetTargetAlphaChannelImageFromTexture() const
{
	return &m_targetTextureSlateBrushes[alphaColorChannel];
}

FReply FWF_TexturePackerModule::OnSourceTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	if (mouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (m_sourceTextureToDisplay)
		{
			GEditor->EditObject(m_sourceTextureToDisplay);
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

FReply FWF_TexturePackerModule::OnSourceRedChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	return OnSourceChannelTextureClicked(myGeometry, mouseEvent, ColorChannel::RED);
}

FReply FWF_TexturePackerModule::OnSourceGreenChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	return OnSourceChannelTextureClicked(myGeometry, mouseEvent, ColorChannel::GREEN);
}

FReply FWF_TexturePackerModule::OnSourceBlueChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	return OnSourceChannelTextureClicked(myGeometry, mouseEvent, ColorChannel::BLUE);
}

FReply FWF_TexturePackerModule::OnSourceAlphaChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	return OnSourceChannelTextureClicked(myGeometry, mouseEvent, ColorChannel::ALPHA);
}

FReply FWF_TexturePackerModule::OnSourceChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent, ColorChannel colorClicked)
{
	int color = (int)colorClicked;

	if (mouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (m_sourceTextureChannels[color])
		{
			GEditor->EditObject(m_sourceTextureChannels[color]);
			return FReply::Handled();
		}
	}

	if (mouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (m_sourceTextureChannels[color])
		{
			TArray< FAssetData > inAssetData;
			inAssetData.Add(FAssetData(m_sourceTextureChannels[color]));
			return FReply::Handled().BeginDragDrop(FAssetDragDropOp::New(inAssetData));
		}
	}

	return FReply::Unhandled();
}

FReply FWF_TexturePackerModule::OnTargetRedChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	return OnTargetChannelTextureClicked(myGeometry, mouseEvent, ColorChannel::RED);
}

FReply FWF_TexturePackerModule::OnTargetGreenChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	return OnTargetChannelTextureClicked(myGeometry, mouseEvent, ColorChannel::GREEN);
}

FReply FWF_TexturePackerModule::OnTargetBlueChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	return OnTargetChannelTextureClicked(myGeometry, mouseEvent, ColorChannel::BLUE);
}

FReply FWF_TexturePackerModule::OnTargetAlphaChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent)
{
	return OnTargetChannelTextureClicked(myGeometry, mouseEvent, ColorChannel::ALPHA);
}

FReply FWF_TexturePackerModule::OnTargetChannelTextureClicked(const FGeometry& myGeometry, const FPointerEvent& mouseEvent, ColorChannel colorClicked)
{
	int color = (int)colorClicked;

	if (mouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (m_targetTextureChannels[color])
		{
			GEditor->EditObject(m_targetTextureChannels[color]);
			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWF_TexturePackerModule, WF_TexturePacker)