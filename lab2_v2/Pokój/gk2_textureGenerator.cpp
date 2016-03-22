#include "gk2_textureGenerator.h"
#include <cmath>
using namespace gk2;

TextureGenerator::TextureGenerator(int octaves, float persistance)
	: m_octaves(octaves), m_persistance(persistance)
{ }

float TextureGenerator::Interpolate(float a, float b, float t)
{
	return b * t + a * (1 - t);
}

float TextureGenerator::Noise1(int x, int y)
{
	unsigned int n = x + y * 73;
	n = (n << 13) ^ n;
	return (1.0f - ((n * (n * n * 37731 + 789223) + 1376312019) & 0x7fffffff) / 2147483647.0f);
}

float TextureGenerator::SmoothNoise1(int x, int y)
{
	float corners = (Noise1(x - 1, y - 1) + Noise1(x + 1, y - 1) + Noise1(x - 1, y + 1) + Noise1(x + 1, y + 1)) / 16;
	float sides = (Noise1(x - 1, y) + Noise1(x + 1, y) + Noise1(x, y - 1) + Noise1(x, y + 1)) / 8;
	float center = Noise1(x, y) / 4;
	return corners + sides + center;
}

float TextureGenerator::InterpolatedNoise1(float x, float y)
{
	int ix = static_cast<int>(x);
	int iy = static_cast<int>(y);
	float v1 = SmoothNoise1(ix, iy);
	float v2 = SmoothNoise1(ix + 1, iy);
	float v3 = SmoothNoise1(ix, iy + 1);
	float v4 = SmoothNoise1(ix + 1, iy + 1);
	float fx = x - ix;
	v1 = Interpolate(v1, v2, fx);
	v3 = Interpolate(v3, v4, fx);
	return Interpolate(v1, v3, y - iy);
}

float TextureGenerator::PerlinNoise2D(float x, float y)
{
	float sum = 0;
	float amplitude = 1;
	float frequency = 1;
	for (int i = 0; i < m_octaves; ++i, amplitude *= m_persistance, frequency *= 2)
		sum += InterpolatedNoise1(x * frequency, y * frequency) * amplitude;
	return sum;
}

float TextureGenerator::Wood(float x, float y)
{
	float g = PerlinNoise2D(x, y) * 25;
	return (g - static_cast<int>(g)) / 4 + 0.7f;
}