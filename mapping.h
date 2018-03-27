#pragma once

#include <thread>
#include <DirectXMath.h>
#include <noise/noise.h>
#include "noiseutils.h"
#include "maths.h"

using namespace DirectX;

class Mapping
{
public:
	Mapping();

	void Setup(int _hres, float m_temp, XMFLOAT3 perlin);
	void CreateMaps(int _hres, float m_temp, XMFLOAT3 perlin);
	void CreateHeightMap(float m_temp, XMFLOAT3 perlin);
	void Shutdown();
	float GetHeightMapValue(int face, int x, int y);
	float GetHeightMapValueFloat(int face, float xCoordFloat, float yCoordFloat);
	XMFLOAT3 GetColorMapValue(int face, int x, int y);
	int GetHeightMapRes();
	bool IsBuilt();
	bool IsBuilding();
private:
	//heightmap
	float* h_map;
	XMFLOAT3* c_map;
	int h_res;
	bool h_built = false;
	bool h_building = false;
};