// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "WF_TexturePacker/Public/WF_TexturePacker.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeWF_TexturePacker() {}
// Cross Module References
	WF_TEXTUREPACKER_API UEnum* Z_Construct_UEnum_WF_TexturePacker_ColorChannel();
	UPackage* Z_Construct_UPackage__Script_WF_TexturePacker();
// End Cross Module References
	static UEnum* ColorChannel_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_WF_TexturePacker_ColorChannel, Z_Construct_UPackage__Script_WF_TexturePacker(), TEXT("ColorChannel"));
		}
		return Singleton;
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_ColorChannel(ColorChannel_StaticEnum, TEXT("/Script/WF_TexturePacker"), TEXT("ColorChannel"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_WF_TexturePacker_ColorChannel_CRC() { return 3284639563U; }
	UEnum* Z_Construct_UEnum_WF_TexturePacker_ColorChannel()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_WF_TexturePacker();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("ColorChannel"), 0, Get_Z_Construct_UEnum_WF_TexturePacker_ColorChannel_CRC(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "ColorChannel::BLUE", (int64)ColorChannel::BLUE },
				{ "ColorChannel::GREEN", (int64)ColorChannel::GREEN },
				{ "ColorChannel::RED", (int64)ColorChannel::RED },
				{ "ColorChannel::ALPHA", (int64)ColorChannel::ALPHA },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "ModuleRelativePath", "Public/WF_TexturePacker.h" },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_WF_TexturePacker,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				"ColorChannel",
				RF_Public|RF_Transient|RF_MarkAsNative,
				nullptr,
				(uint8)UEnum::ECppForm::EnumClass,
				"ColorChannel",
				Enumerators,
				ARRAY_COUNT(Enumerators),
				METADATA_PARAMS(Enum_MetaDataParams, ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
