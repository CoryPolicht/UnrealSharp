#pragma once

#include "CoreMinimal.h"
#include "CSPropertyGenerator.h"
#include "CSArrayPropertyGenerator.generated.h"

UCLASS()
class UNREALSHARPCORE_API UCSArrayPropertyGenerator : public UCSPropertyGenerator
{
	GENERATED_BODY()

protected:

	// Begin UCSPropertyGenerator interface
	virtual ECSPropertyType GetPropertyType() const override { return ECSPropertyType::Array; }
	virtual FFieldClass* GetPropertyClass() override { return FArrayProperty::StaticClass(); }
	virtual FProperty* CreateProperty(UField* Outer, const FCSPropertyMetaData& PropertyMetaData) override;
#if WITH_EDITOR
	virtual void CreatePinInfoEditor(const FCSPropertyMetaData& PropertyMetaData, FEdGraphPinType& PinType) override;
#endif
	// End UCSPropertyGenerator interface
	
};
