#include "CSPropertyFactory.h"
#include "Properties/CSPropertyGenerator.h"
#include "UnrealSharpCore/UnrealSharpCore.h"
#include "UObject/UnrealType.h"
#include "UObject/Class.h"
#include "UnrealSharpCore/TypeGenerator/Register/CSMetaDataUtils.h"
#include "TypeGenerator/Register/MetaData/CSDelegateMetaData.h"

TArray<TWeakObjectPtr<UCSPropertyGenerator>> FCSPropertyFactory::PropertyGenerators;

void FCSPropertyFactory::TryInitializePropertyFactory()
{
	if (PropertyGenerators.Num() > 0)
	{
		return;
	}
	
	TArray<UCSPropertyGenerator*> FoundPropertyGenerators;
	FUnrealSharpCoreModule::GetAllCDOsOfClass<UCSPropertyGenerator>(FoundPropertyGenerators);

	for (UCSPropertyGenerator* PropertyGenerator : FoundPropertyGenerators)
	{
		PropertyGenerators.Add(PropertyGenerator);
	}
}

FProperty* FCSPropertyFactory::CreateProperty(UField* Outer, const FCSPropertyMetaData& PropertyMetaData)
{
	TryInitializePropertyFactory();
	
	UCSPropertyGenerator* PropertyGenerator = FindPropertyGenerator(PropertyMetaData.Type->PropertyType);
	FProperty* NewProperty = PropertyGenerator->CreateProperty(Outer, PropertyMetaData);

	NewProperty->SetPropertyFlags(PropertyMetaData.PropertyFlags);
	NewProperty->SetBlueprintReplicationCondition(PropertyMetaData.LifetimeCondition);

#if WITH_EDITOR
	if (!PropertyMetaData.BlueprintSetter.IsEmpty())
	{
		NewProperty->SetMetaData("BlueprintSetter", *PropertyMetaData.BlueprintSetter);

		if (UFunction* BlueprintSetterFunction = CastChecked<UClass>(Outer)->FindFunctionByName(*PropertyMetaData.BlueprintSetter))
		{
			BlueprintSetterFunction->SetMetaData("BlueprintInternalUseOnly", TEXT("true"));
		}
	}

	if (!PropertyMetaData.BlueprintGetter.IsEmpty())
	{
		NewProperty->SetMetaData("BlueprintGetter", *PropertyMetaData.BlueprintGetter);
			
		if (UFunction* BlueprintGetterFunction = CastChecked<UClass>(Outer)->FindFunctionByName(*PropertyMetaData.BlueprintGetter))
		{
			BlueprintGetterFunction->SetMetaData("BlueprintInternalUseOnly", TEXT("true"));
		}
	}
#endif

	if (NewProperty->HasAnyPropertyFlags(CPF_Net))
	{
		UBlueprintGeneratedClass* OwnerClass = CastChecked<UBlueprintGeneratedClass>(Outer);
		++OwnerClass->NumReplicatedProperties;
			
		if (!PropertyMetaData.RepNotifyFunctionName.IsNone())
		{
			NewProperty->RepNotifyFunc = PropertyMetaData.RepNotifyFunctionName;
			NewProperty->SetPropertyFlags(CPF_Net | CPF_RepNotify);
		}
	}
		
	FCSMetaDataUtils::ApplyMetaData(PropertyMetaData.MetaData, NewProperty);
		
	return NewProperty;
}

FProperty* FCSPropertyFactory::CreateAndAssignProperty(UField* Outer, const FCSPropertyMetaData& PropertyMetaData)
{
	FProperty* Property = CreateProperty(Outer, PropertyMetaData);
	Outer->AddCppProperty(Property);
	PropertyMetaData.Type->OnPropertyCreated(Property);
	return Property;
}

void FCSPropertyFactory::CreateAndAssignProperties(UField* Outer, const TArray<FCSPropertyMetaData>& PropertyMetaData)
{
	for (const FCSPropertyMetaData& Property : PropertyMetaData)
	{
		CreateAndAssignProperty(Outer, Property);
	}
}

UCSPropertyGenerator* FCSPropertyFactory::FindPropertyGenerator(ECSPropertyType PropertyType)
{
	TryInitializePropertyFactory();
	
	for (TWeakObjectPtr<UCSPropertyGenerator>& PropertyGenerator : PropertyGenerators)
	{
		UCSPropertyGenerator* PropertyGeneratorPtr = PropertyGenerator.Get();
		if (!PropertyGeneratorPtr->SupportsPropertyType(PropertyType))
		{
			continue;
		}

		return PropertyGeneratorPtr;
	}
	
	return nullptr;
}

void FCSPropertyFactory::CreateAndAssignPropertiesEditor(UBlueprint* Outer, const TArray<FCSPropertyMetaData>& PropertyMetaData)
{
	Outer->NewVariables.Empty();
	
	for (const FCSPropertyMetaData& Property : PropertyMetaData)
	{
		UCSPropertyGenerator* PropertyGenerator = FindPropertyGenerator(Property.Type->PropertyType);

		if (!IsValid(PropertyGenerator))
		{
			continue;
		}
		
		PropertyGenerator->CreatePropertyEditor(Outer, Property);
	}
}


