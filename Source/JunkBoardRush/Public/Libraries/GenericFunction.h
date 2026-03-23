
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GenericFunction.generated.h"


UCLASS()
class JUNKBOARDRUSH_API UGenericFunction : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template<typename T>
	static const T& GetDataRow(const FDataTableRowHandle& Handle, const FString& Context)
	{
		if (Handle.DataTable && !Handle.RowName.IsNone())
		{
			if (const T* Row = Handle.GetRow<T>(Context))
			{
				return *Row;
			}
		}

		UE_LOG(LogTemp, Error, TEXT("Row not found for context '%s'. Returning default."), *Context);
        
		static const T DefaultData{};
		return DefaultData;
	}
};
