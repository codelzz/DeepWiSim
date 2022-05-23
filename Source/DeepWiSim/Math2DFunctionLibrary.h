// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math2DFunctionLibrary.generated.h"


USTRUCT(BlueprintType)
struct FFloatArray1D
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Math2D|Array1D")
		TArray<float> Array;

public:
	/** Init with Size of first dimension. */
	void Init(int32 Num) { Array.Init(0, Num); }

	/** Init with same shape as given Array. */
	void InitLike(const FFloatArray1D& Other) { Array.Init(0, Other.Num()); }

	bool Like(const FFloatArray1D& Other) { return Num() == Other.Num(); }

	/** Return Num of element in Array1D. */
	int32 Num() const {return Array.Num();}

	/** Check if Array is empty. */
	bool IsEmpty() const {return Num() == 0;}
	
	/** Return String Representation of Array1D. */
	FString ToString() const
	{		
		if (Num() == 0) {return TEXT("[]");}
		FString ArrayString = TEXT("[");
		for (const float Float : Array)
		{
			ArrayString += FString::Printf(TEXT("%.4f "), Float);
		}
		ArrayString = ArrayString.LeftChop(1) + TEXT("]");
		return ArrayString;
	}

	/** Return the max value in Array. */
	float Max() const { return FMath::Max(Array); }

	/** Return the min value in Array. */
	float Min() const { return FMath::Min(Array); }

	//~ begin Operator Overload
	/* Array subscript operator. */
	float& operator[](std::size_t Index) { return Array[Index]; }
	const float& operator[](std::size_t Index) const { return Array[Index]; }

	/* Binary arithmetic operators. */
	/* += */
	FFloatArray1D& operator+=(const FFloatArray1D& RHS)
	{
		check(this->Like(RHS));
		for (int32 idx = 0; idx < Num(); idx++)
		{
			Array[idx] += RHS[idx];
		}
		return *this;
	}
	/* + */
	friend FFloatArray1D operator+(FFloatArray1D LHS, const FFloatArray1D& RHS)
	{
		check(LHS.Like(RHS));
		LHS += RHS;
		return LHS;
	}
	//~ end Operator Overload
};

USTRUCT(BlueprintType)
struct FFloatArray2D
{
	GENERATED_BODY()
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Math2D|Array2D")
		TArray<FFloatArray1D> Array;

public:
	/** Init with shape */
	void Init(int32 Num0, int32 Num1)
	{
		Array.Init(FFloatArray1D(), Num0);
		for (FFloatArray1D& Array1D : Array){Array1D.Init(Num1);}
	}

	/** Init with same shape as given Array. */
	void InitLike(const FFloatArray2D& Other)
	{
		Array.Init(FFloatArray1D(), Other.Num0());
		for (FFloatArray1D& Array1D : Array){Array1D.Init(Other.Num1());}
	}

	/** Check if two 2D array has same shape */
	bool Like(const FFloatArray2D& Other) { return Num0() == Other.Num0() && Num1() == Other.Num1(); }

	/** Get the size of first dimension */
	int32 Num0() const {return Array.Num();}

	/** Get the size of second dimension */
	int32 Num1() const {return (Num0() == 0)? 0 : Array[0].Num();}

	/** Check if array is empty */
	bool IsEmpty() const {return Num0() == 0 || Num1() == 0;}

	/** Convert array to string representation */
	FString ToString() const
	{
		if (Num0() == 0) { return TEXT("[]"); }
		FString Array2DString = TEXT("[");
		int32 LineNum = 1;
		int32 MaxLineNum = Num0();
		for (auto Array1D : Array)
		{
			FString Array1DString = Array1D.ToString();
			// if is not the first line, append 'SPACE' at the beginning
			if (LineNum != 1) {Array1DString = TEXT(" ") + Array1DString;}
			// if is not the last line, append ',\n' at the end
			if (LineNum != MaxLineNum) {Array1DString += TEXT(",\n");}
			Array2DString += Array1DString;
			LineNum++;
		}
		return Array2DString + TEXT("]");
	}

	/** Get the Transpose of 2D array */
	FFloatArray2D Transpose() const
	{
		FFloatArray2D Result = FFloatArray2D();
		if (IsEmpty()) { return Result; };
		Result.Init(Num1(), Num0());
		for (int32 Index0 = 0; Index0 < Num0(); Index0++)
		{
			for (int32 Index1 = 0; Index1 < Num1(); Index1++)
			{
				Result.Array[Index1].Array[Index0] = Array[Index0].Array[Index1];
			}
		}
		return Result;
	}

	/** ReduceMax */
	FFloatArray1D ReduceMax(int32 Dim) const
	{
		check(Dim < 2 && Dim >= 0);
		FFloatArray2D ArrayT = (Dim == 0) ? *this : this->Transpose();
		FFloatArray1D Result;
		Result.Init(ArrayT.Num0());
		for (int32 Index = 0; Index < ArrayT.Num0(); Index++)
		{
			Result[Index] = ArrayT[Index].Max();
		}
		return Result;
	}
public:
	//~ begin Operator Overload
	/* Array subscript operator. */
	FFloatArray1D& operator[](std::size_t idx) { return Array[idx]; }
	const FFloatArray1D& operator[](std::size_t idx) const { return Array[idx]; }

	/* Binary arithmetic operators. */
	/* += */
	FFloatArray2D& operator+=(const FFloatArray2D& RHS)
	{
		check(this->Like(RHS));
		for (int32 idx = 0; idx < Num0(); idx++)
		{
			Array[idx] += RHS[idx];
		}
		return *this;
	}
	/* + */
	friend FFloatArray2D operator+(FFloatArray2D LHS, const FFloatArray2D& RHS)
	{
		check(LHS.Like(RHS)) ;
		LHS += RHS;
		return LHS;
	}
	//~ end Operator Overload
};

/**
 * 
 */
UCLASS(Blueprintable)
class DEEPWISIM_API UMath2DFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};
