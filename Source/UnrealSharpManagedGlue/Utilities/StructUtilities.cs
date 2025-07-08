﻿using EpicGames.Core;
using EpicGames.UHT.Types;
using UnrealSharpScriptGenerator.PropertyTranslators;

namespace UnrealSharpScriptGenerator.Utilities;

public static class StructUtilities
{
    public static bool IsStructBlittable(this UhtStruct structObj)
    {
        if (PropertyTranslatorManager.BlittableTypes.Contains(structObj.SourceName))
        {
            return true;
        }
        
        // Any struct we haven't manually exported is not blittable, yet.
        // The fix for this is to add a header parser to check for non-UPROPERTY properties in the struct.
        // Because a struct can be recognized as blittable by the reflection data,
        // but have a non-UPROPERTY property that is not picked up by UHT, that makes it not blittable causing a mismatch in memory layout.
        // This is a temporary solution until we can get that working.
        return false;
    }

    public static bool IsStructNativelyCopyable(this UhtStruct structObj)
    {
        return PropertyTranslatorManager.NativelyCopyableTypes.Contains(structObj.SourceName);
    }
    
    public static bool IsStructNativelyDestructible(this UhtStruct structObj)
    {
        return PropertyTranslatorManager.NativelyDestructibleTypes.Contains(structObj.SourceName);
    }
}