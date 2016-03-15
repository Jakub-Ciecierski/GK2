#ifndef __GK2_TEXTURE_GENERATOR_H_
#define __GK2_TEXTURE_GENERATOR_H_

namespace gk2
{
	class TextureGenerator
	{
	public:
		TextureGenerator(int octaves, float persistance);

		float PerlinNoise2D(float x, float y);
		float Wood(float x, float y);

	private:
		int m_octaves;
		float m_persistance;

		float Noise1(int x, int y);
		float SmoothNoise1(int x, int y);
		float InterpolatedNoise1(float x, float y);
		float Interpolate(float a, float b, float t);
	};
}

#endif __GK2_TEXTURE_GENERATOR_H_
