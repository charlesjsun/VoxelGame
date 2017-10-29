// 
// This is a Simplex Noise algorithm released into the public domain by Stefan Gustavson
// Original Source can be found here: http://staffwww.itn.liu.se/~stegu/aqsis/aqsis-newnoise/
//
// Modified to take in different variables and return different types of noise.
//

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimplexNoise.generated.h"

/**
 * 
 */
UCLASS()
class AETHERIAGAME_API USimplexNoise : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/** Scales the noise value from -1 -> 1 to Min -> Max */
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float ScaleNoise(float Value, float Min, float Max);

	/** @returns the noise function at x */
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float Noise1D(float x);

	/** @returns the 2D noise function at <x, y> */
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float Noise2D(float x, float y);

	/** @returns the 3D noise function at <x, y, z> */
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float Noise3D(float x, float y, float z);

	/** @returns the 4D noise function at <x, y, z, w> */
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float Noise4D(float x, float y, float z, float w);

	/**
	*  @param x - x coordinate.
	*  @param Octaves - Number of octaves. Must be bigger than 0.
	*  @param Frequency - How frequent the Noise function is called. Default 1, higher = more rise and falls.
	*  @param Lacunarity - How quickly frequency is increased every octave. Frequency is multiplied by this every octave.
	*  @param Persistence - How quickly amplitude is decreased every octave. Amplitude is multiplied by this every octave. Less than 1;
	*  @returns the fBm fractal function at x.
	*/
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float Fractal1D(float x, int32 Octaves = 1, float Frequency = 1.0f, float Lacunarity = 2.0f, float Persistence = 0.5f);

	/**
	*  @param x - x Coordinate.
	*  @param y - y Coordinate.
	*  @param Octaves - Number of octaves. Must be bigger than 0.
	*  @param Frequency - How frequent the Noise function is called. Default 1, higher = more rise and falls.
	*  @param Lacunarity - How quickly frequency is increased every octave. Frequency is multiplied by this every octave.
	*  @param Persistence - How quickly amplitude is decreased every octave. Amplitude is multiplied by this every octave. Less than 1;
	*  @returns the fBm fractal function at <x, y>.
	*/
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float Fractal2D(float x, float y, int32 Octaves = 1, float Frequency = 1.0f, float Lacunarity = 2.0f, float Persistence = 0.5f);

	/**
	*  @param x - x Coordinate.
	*  @param y - y Coordinate.
	*  @param z - z Coordinate.
	*  @param Octaves - Number of octaves. Must be bigger than 0.
	*  @param Frequency - How frequent the Noise function is called. Default 1, higher = more rise and falls.
	*  @param Lacunarity - How quickly frequency is increased every octave. Frequency is multiplied by this every octave.
	*  @param Persistence - How quickly amplitude is decreased every octave. Amplitude is multiplied by this every octave. Less than 1;
	*  @returns the fBm fractal function at <x, y, z>.
	*/
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float Fractal3D(float x, float y, float z, int32 Octaves = 1, float Frequency = 1.0f, float Lacunarity = 2.0f, float Persistence = 0.5f);

	/**
	*  @param x - x Coordinate.
	*  @param y - y Coordinate.
	*  @param z - z Coordinate.
	*  @param w - w Coordinate.
	*  @param Octaves - Number of octaves. Must be bigger than 0.
	*  @param Frequency - How frequent the Noise function is called. Default 1, higher = more rise and falls.
	*  @param Lacunarity - How quickly frequency is increased every octave. Frequency is multiplied by this every octave.
	*  @param Persistence - How quickly amplitude is decreased every octave. Amplitude is multiplied by this every octave. Less than 1;
	*  @returns the fBm fractal function at <x, y, z, w>.
	*/
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float Fractal4D(float x, float y, float z, float w, int32 Octaves = 1, float Frequency = 1.0f, float Lacunarity = 2.0f, float Persistence = 0.5f);

	/**
	*  @param x - x coordinate.
	*  @param Octaves - Number of octaves. Must be bigger than 0.
	*  @param Frequency - How frequent the Noise function is called. Default 1, higher = more rise and falls.
	*  @param Lacunarity - How quickly frequency is increased every octave. Frequency is multiplied by this every octave.
	*  @returns the RigedMulti fractal function at x.
	*/
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float RidgedMulti1D(float x, int32 Octaves = 1, float Frequency = 1.0f, float Lacunarity = 2.0f);

	/**
	*  @param x - x Coordinate.
	*  @param y - y Coordinate.
	*  @param Octaves - Number of octaves. Must be bigger than 0.
	*  @param Frequency - How frequent the Noise function is called. Default 1, higher = more rise and falls.
	*  @param Lacunarity - How quickly frequency is increased every octave. Frequency is multiplied by this every octave.
	*  @returns the RigedMulti fractal function at <x, y>.
	*/
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float RidgedMulti2D(float x, float y, int32 Octaves = 1, float Frequency = 1.0f, float Lacunarity = 2.0f);

	/**
	*  @param x - x Coordinate.
	*  @param y - y Coordinate.
	*  @param z - z Coordinate.
	*  @param Octave - Number of octaves. Must be bigger than 0.
	*  @param Frequency - How frequent the Noise function is called. Default 1, higher = more rise and falls.
	*  @param Lacunarity - How quickly frequency is increased every octave. Frequency is multiplied by this every octave.
	*  @returns the RigedMulti fractal function at <x, y, z>.
	*/
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float RidgedMulti3D(float x, float y, float z, int32 Octaves = 1, float Frequency = 1.0f, float Lacunarity = 2.0f);

	/**
	*  @param x - x Coordinate.
	*  @param y - y Coordinate.
	*  @param z - z Coordinate.
	*  @param w - w Coordinate.
	*  @param Octaves - Number of octaves. Must be bigger than 0.
	*  @param Frequency - How frequent the Noise function is called. Default 1, higher = more rise and falls.
	*  @param Lacunarity - How quickly frequency is increased every octave. Frequency is multiplied by this every octave.
	*  @returns the RigedMulti fractal function at <x, y, z, w>.
	*/
	UFUNCTION(Category = "Simplex Noise", BlueprintCallable, BlueprintPure)
	static float RidgedMulti4D(float x, float y, float z, float w, int32 Octaves = 1, float Frequency = 1.0f, float Lacunarity = 2.0f);

	
	
};
