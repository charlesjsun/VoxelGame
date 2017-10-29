// Copyright (c) 2016-2017, Charles JL Sun, All rights reserved.

#include "SimplexNoise.h"

#include "SimplexNoiseImplementation.inl"

//////////////////////////////////////////////////////////////////
/// UTILILIES
//////////////////////////////////////////////////////////////////
float USimplexNoise::ScaleNoise(float Value, float Min, float Max)
{
	return (0.5f - Value * 0.5f) * Min + (Value * 0.5f + 0.5f) * Max;
}

//////////////////////////////////////////////////////////////////
/// SIMPLEX NOISE
//////////////////////////////////////////////////////////////////

float USimplexNoise::Noise1D(float x)
{
	return SimplexNoiseImplementation::noise(x);
}

float USimplexNoise::Noise2D(float x, float y)
{
	return SimplexNoiseImplementation::noise(x, y);
}

float USimplexNoise::Noise3D(float x, float y, float z)
{
	return SimplexNoiseImplementation::noise(x, y, z);
}

float USimplexNoise::Noise4D(float x, float y, float z, float w)
{
	return SimplexNoiseImplementation::noise(x, y, z, w);
}


//////////////////////////////////////////////////////////////////
/// FRACTALS
//////////////////////////////////////////////////////////////////

float USimplexNoise::Fractal1D(float x, int32 Octaves, float Frequency, float Lacunarity, float Persistence)
{
	float Value = 0.0f;
	float Amplitude = 1.0f;
	x *= Frequency;
	for (int32 i = 0; i < Octaves; i++)
	{
		float Noise = Noise1D(x);
		Value += Noise * Amplitude;
		x *= Lacunarity;
		Amplitude *= Persistence;
	}
	return Value;
}

float USimplexNoise::Fractal2D(float x, float y, int32 Octaves, float Frequency, float Lacunarity, float Persistence)
{
	float Value = 0.0f;
	float Amplitude = 1.0f;
	x *= Frequency;
	y *= Frequency;
	for (int32 i = 0; i < Octaves; i++)
	{
		float Noise = Noise2D(x, y);
		Value += Noise * Amplitude;
		x *= Lacunarity;
		y *= Lacunarity;
		Amplitude *= Persistence;
	}
	return Value;
}

float USimplexNoise::Fractal3D(float x, float y, float z, int32 Octaves, float Frequency, float Lacunarity, float Persistence)
{
	float Value = 0.0f;
	float Amplitude = 1.0f;
	x *= Frequency;
	y *= Frequency;
	z *= Frequency;
	for (int32 i = 0; i < Octaves; i++)
	{
		float Noise = Noise3D(x, y, z);
		Value += Noise * Amplitude;
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;
		Amplitude *= Persistence;
	}
	return Value;
}

float USimplexNoise::Fractal4D(float x, float y, float z, float w, int32 Octaves, float Frequency, float Lacunarity, float Persistence)
{
	float Value = 0.0f;
	float Amplitude = 1.0f;
	x *= Frequency;
	y *= Frequency;
	z *= Frequency;
	w *= Frequency;
	for (int32 i = 0; i < Octaves; i++)
	{
		float Noise = Noise4D(x, y, z, w);
		Value += Noise * Amplitude;
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;
		w *= Lacunarity;
		Amplitude *= Persistence;
	}
	return Value;
}


//////////////////////////////////////////////////////////////////
/// RIDGEDMULTI
//////////////////////////////////////////////////////////////////

float USimplexNoise::RidgedMulti1D(float x, int32 Octaves, float Frequency, float Lacunarity)
{
	x *= Frequency;
	float Value = 0.0f;
	float Weight = 1.0f;
	float Offset = 1.0f;
	float Gain = 2.0f;
	float Amplitude = 1.0f;
	float Persistence = 1.0f / Lacunarity;
	for (int32 i = 0; i < Octaves; i++)
	{
		float Noise = Noise1D(x);
		Noise = FMath::Abs(Noise);
		Noise = Offset - Noise;
		Noise *= Noise;
		Noise *= Weight;
		Weight = Noise * Gain;
		Weight = FMath::Clamp(Weight, 0.0f, 1.0f);
		Value += Noise * Amplitude;
		x *= Lacunarity;
		Amplitude *= Persistence;

	}
	return (Value * 1.25f) - 1.0f;
}

float USimplexNoise::RidgedMulti2D(float x, float y, int32 Octaves, float Frequency, float Lacunarity)
{
	x *= Frequency;
	y *= Frequency;
	float Value = 0.0f;
	float Weight = 1.0f;
	float Offset = 1.0f;
	float Gain = 2.0f;
	float Amplitude = 1.0f;
	float Persistence = 1.0f / Lacunarity;
	for (int32 i = 0; i < Octaves; i++)
	{
		float Noise = Noise2D(x, y);
		Noise = FMath::Abs(Noise);
		Noise = Offset - Noise;
		Noise *= Noise;
		Noise *= Weight;
		Weight = Noise * Gain;
		Weight = FMath::Clamp(Weight, 0.0f, 1.0f);
		Value += Noise * Amplitude;
		x *= Lacunarity;
		y *= Lacunarity;
		Amplitude *= Persistence;
	}
	return (Value * 1.25f) - 1.0f;
}

float USimplexNoise::RidgedMulti3D(float x, float y, float z, int32 Octaves, float Frequency, float Lacunarity)
{
	x *= Frequency;
	y *= Frequency;
	z *= Frequency;
	float Value = 0.0f;
	float Weight = 1.0f;
	float Offset = 1.0f;
	float Gain = 2.0f;
	float Amplitude = 1.0f;
	float Persistence = 1.0f / Lacunarity;
	for (int32 i = 0; i < Octaves; i++)
	{
		float Noise = Noise3D(x, y, z);
		Noise = FMath::Abs(Noise);
		Noise = Offset - Noise;
		Noise *= Noise;
		Noise *= Weight;
		Weight = Noise * Gain;
		Weight = FMath::Clamp(Weight, 0.0f, 1.0f);
		Value += Noise * Amplitude;
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;
		Amplitude *= Persistence;
	}
	return (Value * 1.25f) - 1.0f;
}

float USimplexNoise::RidgedMulti4D(float x, float y, float z, float w, int32 Octaves, float Frequency, float Lacunarity)
{
	x *= Frequency;
	y *= Frequency;
	z *= Frequency;
	w *= Frequency;
	float Value = 0.0f;
	float Weight = 1.0f;
	float Offset = 1.0f;
	float Gain = 2.0f;
	float Amplitude = 1.0f;
	float Persistence = 1.0f / Lacunarity;
	for (int32 i = 0; i < Octaves; i++)
	{
		float Noise = Noise4D(x, y, z, w);
		Noise = FMath::Abs(Noise);
		Noise = Offset - Noise;
		Noise *= Noise;
		Noise *= Weight;
		Weight = Noise * Gain;
		Weight = FMath::Clamp(Weight, 0.0f, 1.0f);
		Value += Noise * Amplitude;
		x *= Lacunarity;
		y *= Lacunarity;
		z *= Lacunarity;
		w *= Lacunarity;
		Amplitude *= Persistence;
	}
	return (Value * 1.25f) - 1.0f;
}


