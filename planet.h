#pragma once

#include <thread>

#include "face.h"
#include "mapping.h"
#include "Star.h"

class Planet
{
public:
	Planet();
	~Planet();

	void Initialize(XMFLOAT3 position, float size, XMFLOAT3 perlin, XMFLOAT4 sky, ID3D11Device* device, ID3D11DeviceContext* context, StarParam star);
	void Setup(XMFLOAT3 position, float size, XMFLOAT3 perlin, XMFLOAT4 sky, ID3D11Device* device, ID3D11DeviceContext* context, StarParam star);
	std::list<ModelClass*> GetModels(XMFLOAT3 camPos, ID3D11Device* device, ID3D11DeviceContext* context);
	void Shutdown();
	bool DrawSky(XMFLOAT3 camPos);
	XMFLOAT4 GetSky();
	XMFLOAT3 GetPosition();
	float GetSize();

private:
	Face* m_faces[6];
	//need a pointer to the star, or the temperature
	//also the colour scheme to be used

	XMFLOAT3 m_pos;
	float m_size;
	float tempDistance;
	float m_temp;
	XMFLOAT3 m_perlin;

	//the sky colour, if it has one
	XMFLOAT4 m_sky;

	//heightmap
	int h_res;
	bool h_built;
	bool hires;

	Mapping* m_map;
	Mapping* hires_map;
};