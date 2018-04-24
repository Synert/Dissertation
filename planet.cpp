#include "planet.h"

Planet::Planet()
{
	h_built = false;
	hires = false;
}

Planet::~Planet()
{

}

void Planet::Initialize(XMFLOAT3 position, float size, XMFLOAT3 perlin, XMFLOAT3 mapPerlin, XMFLOAT4 sky, ID3D11Device* device, ID3D11DeviceContext* context,
	StarParam star, Mapping* hires_map, float waterHeight, float flatten, float temperature)
{
	h_built = false;
	m_map = new Mapping();
	hires = false;

	m_perlin = perlin;
	m_mapPerlin = mapPerlin;
	m_sky = sky;
	m_size = size;
	m_pos = position;
	m_waterHeight = waterHeight;
	m_flatten = flatten;

	std::thread new_thread(&Planet::Setup, this, position, size, perlin, mapPerlin, sky, device, context, star, hires_map, waterHeight, flatten, temperature);
	new_thread.detach();
	//new_thread.join();
}

void Planet::Setup(XMFLOAT3 position, float size, XMFLOAT3 perlin, XMFLOAT3 mapPerlin, XMFLOAT4 sky, ID3D11Device* device, ID3D11DeviceContext* context,
	StarParam star, Mapping* hires_map, float waterHeight, float flatten, float temperature)
{
	//calculation for the average surface temperature

	XMVECTOR _pos = XMLoadFloat3(&position);
	XMVECTOR _spos = XMLoadFloat3(&star.pos);
	float p_distance;
	_pos -= _spos;
	_pos = XMVector3Length(_pos);
	XMStoreFloat(&p_distance, _pos);
	p_distance *= 200000.0f;
	float albedo = 0.0f;

	m_temp = star.temperature * powf(1.0f - albedo, 0.25f) * powf(star.radius / (2.0f * p_distance), 0.5f);

	if (temperature != -1) m_temp = temperature;

	m_map->CreateMaps(256, m_temp, mapPerlin, waterHeight, flatten);

	Face::Transform tempTransform;
	tempTransform = Face::Transform(position, XMFLOAT3(size, size, size), XMFLOAT3(0.0f, 0.0f, 0.0f), perlin);
	for (int i = 0; i < 6; i++)
	{
		m_faces[i] = new Face();
		m_faces[i]->Initialize(tempTransform, 6, i, (size * 5.0f), device, context, XMFLOAT3(-1.0f, -1.0f, 0.0f), 1.0f, 8, m_map, hires_map, this);
	}

	h_built = true;
}

std::list<ModelClass*> Planet::GetModels(XMFLOAT3 camPos, ID3D11Device* device, ID3D11DeviceContext* context, Mapping* hires_map)
{
	std::list<ModelClass*> result;

	if (!h_built) return result;

	for (int i = 0; i < 6; i++)
	{
		std::list<ModelClass*> tempList = m_faces[i]->GetModels(camPos, device, context);
		for each(ModelClass* model in tempList)
		{
			result.push_back(model);
		}
	}

	return result;
}

bool Planet::Shutdown()
{
	//first, make sure the heightmaps aren't currently generating or anything goofy

	if (!h_built) return false;

	h_built = false;

	for (int i = 0; i < 6; i++)
	{
		if(!m_faces[i]->Shutdown()) return false;
		delete m_faces[i];
	}

	if(!m_map->Shutdown()) return false;
	delete m_map;

	return true;
}

bool Planet::DrawSky(XMFLOAT3 camPos)
{
	if (!h_built) return false;

	XMVECTOR mPos = XMLoadFloat3(&m_pos);
	XMVECTOR cPos = XMLoadFloat3(&camPos);
	float distance;
	mPos -= cPos;
	mPos = XMVector3Length(mPos);
	XMStoreFloat(&distance, mPos);

	tempDistance = distance;

	if (distance < m_size * 2.0f)
	{

		return true;
	}

	return false;
}

XMFLOAT4 Planet::GetSky()
{
	if (!h_built) return XMFLOAT4(0, 0, 0, 0);

	XMFLOAT4 result = m_sky;

	float scalar = (m_size * 1 - tempDistance) / (m_size * 1);

	result.x *= scalar;
	result.y *= scalar;
	result.z *= scalar;

	return result;
}

XMFLOAT4 Planet::GetUnscaledSky()
{
	return m_sky;
}

XMFLOAT3 Planet::GetPosition()
{
	if (!h_built) return XMFLOAT3(0, 0, 0);

	return m_pos;
}

float Planet::GetSize()
{
	if (!h_built) return 0.0f;

	return m_size;
}

float Planet::GetTemperature()
{
	return m_temp;
}

float Planet::GetWaterHeight()
{
	return m_waterHeight;
}

XMFLOAT3 Planet::GetPerlin()
{
	return m_perlin;
}

XMFLOAT3 Planet::GetMapPerlin()
{
	return m_mapPerlin;
}

bool Planet::Built()
{
	return h_built;
}

float Planet::GetFlat()
{
	return m_flatten;
}