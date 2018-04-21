#include "planet.h"

Planet::Planet()
{
	h_built = false;
	//m_map = new Mapping();
	//hires_map = new Mapping();
	hires = false;
}

Planet::~Planet()
{

}

void Planet::Initialize(XMFLOAT3 position, float size, XMFLOAT3 perlin, XMFLOAT4 sky, ID3D11Device* device, ID3D11DeviceContext* context, StarParam star)
{
	h_built = false;
	m_map = new Mapping();
	hires_map = new Mapping();
	hires = false;

	m_perlin = perlin;
	m_sky = sky;
	m_size = size;
	m_pos = position;

	std::thread new_thread(&Planet::Setup, this, position, size, perlin, sky, device, context, star);
	new_thread.detach();
}

void Planet::Setup(XMFLOAT3 position, float size, XMFLOAT3 perlin, XMFLOAT4 sky, ID3D11Device* device, ID3D11DeviceContext* context, StarParam star)
{
	//calculation for the average surface temperature

	XMVECTOR _pos = XMLoadFloat3(&position);
	XMVECTOR _spos = XMLoadFloat3(&star.pos);
	float p_distance;
	_pos -= _spos;
	_pos = XMVector3Length(_pos);
	XMStoreFloat(&p_distance, _pos);
	p_distance *= 300000.0f;
	float albedo = 0.0f;

	m_temp = star.temperature * powf(1.0f - albedo, 0.25f) * powf(star.radius / (2.0f * p_distance), 0.5f);

	m_map->CreateMaps(256, m_temp, perlin);

	Face::Transform tempTransform;
	tempTransform = Face::Transform(position, XMFLOAT3(size, size, size), XMFLOAT3(0.0f, 0.0f, 0.0f), perlin);
	for (int i = 0; i < 6; i++)
	{
		m_faces[i] = new Face();
		m_faces[i]->Initialize(tempTransform, 6, i, (size * 5.0f), device, context, XMFLOAT3(-1.0f, -1.0f, 0.0f), 1.0f, 4, m_map, hires_map);
	}

	h_built = true;
}

std::list<ModelClass*> Planet::GetModels(XMFLOAT3 camPos, ID3D11Device* device, ID3D11DeviceContext* context)
{
	if (hires_map->IsBuilt() && !hires)
	{
		hires = true;
		for (int i = 0; i < 6; i++)
		{
			//m_faces[i]->Rebuild(device, context);
		}
	}

	if (!hires_map->IsBuilt() && hires)
	{
		hires = false;
		for (int i = 0; i < 6; i++)
		{
			//m_faces[i]->Rebuild(device, context);
		}
	}

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

void Planet::Shutdown()
{
	//first, make sure the heightmaps aren't currently generating or anything goofy

	for (int i = 0; i < 6; i++)
	{
		if (m_faces[i] != NULL)
		{
			m_faces[i]->Shutdown();
			delete m_faces[i];
			m_faces[i] = 0;
		}
	}

	m_map->Shutdown();
	delete m_map;
	m_map = 0;

	hires_map->Shutdown();
	delete hires_map;
	hires_map = 0;
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
		//if it's close enough for sky, it's also close enough for the hi-res

		if (!hires_map->IsBuilding())
		{
			hires_map->Setup(1024, m_temp, m_perlin);
		}

		return true;
	}

	else
	{
		if (hires_map->IsBuilt())
		{
			hires_map->Shutdown();
			//delete hires_map;
			hires = false;
		}
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