#pragma once

#include <thread>
#include <DirectXMath.h>
#include <noise/noise.h>
#include "noiseutils.h"
#include "maths.h"

using namespace DirectX;

enum Biome
{
	BIOME_NONE,
	BIOME_TUNDRA,
	BIOME_SNOW,
	BIOME_DESERT_COLD,
	BIOME_DESERT_HOT,
	BIOME_FOREST_BOREAL,
	BIOME_FOREST_TEMPERATE_RAIN,
	BIOME_FOREST_TEMPERATE_SEASONAL,
	BIOME_FOREST_TROPICAL_RAIN,
	BIOME_FOREST_TROPICAL_SEASONAL,
	BIOME_WOODLAND
};

class Mapping
{
public:
	Mapping();

	void Setup(int _hres, float m_temp, XMFLOAT3 perlin, float waterHeight, float flatten);
	void CreateMaps(int _hres, float m_temp, XMFLOAT3 perlin, float waterHeight, float flatten);
	void CreateHeightMap(float m_temp, XMFLOAT3 perlin, float waterHeight, float flatten);
	void HeightmapThread(int z, float temp, XMFLOAT3 perlin, float waterHeight, float flatten);
	bool Shutdown();
	float GetHeightMapValue(int face, int x, int y);
	float GetHeightMapValueFloat(int face, float xCoordFloat, float yCoordFloat);
	XMFLOAT3 GetColorMapValue(int face, int x, int y);
	int GetHeightMapRes();
	bool IsBuilt();
	bool IsBuilding();
	void Cancel();
	bool Cancelled();

	XMFLOAT3 Blend(XMFLOAT3 a, XMFLOAT3 b);

	float GetWaterHeight();
	void SetPlanet(void* planet);
	void* CurrentPlanet();
private:
	//heightmap
	float* h_map;
	XMFLOAT3* c_map;
	int h_res;
	bool h_built = false;
	bool h_building = false;
	bool cancel = false;
	int h_finished = 0;

	float m_waterHeight;
	void* m_planet;
};