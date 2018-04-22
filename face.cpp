#include "face.h"

Face::Face()
{
	hasHires = false;
	created = false;
	creating = false;
	m_parent = 0;
}

Face::~Face()
{

}

bool Face::Initialize(Transform transform, int recursion, int faceDir, float distance,
	ID3D11Device* device, ID3D11DeviceContext* context, XMFLOAT3 offset, float size,
	int divide, Mapping* map, Mapping* hires, void* planet)
{
	creating = true;

	hasHires = false;

	m_planet = planet;

	m_transform = transform;
	m_faceDir = faceDir;
	m_distance = distance;
	m_offset = offset;
	m_size = size;
	m_recursion = recursion;
	m_divide = divide;
	//m_temperature = temperature;
	//m_temperature = 400;
	//m_map = h_map;
	//m_res = h_res;

	float temperature = 400;

	m_map = map;
	lowres_map = map;
	hires_map = hires;

	if (hires_map->IsBuilt() && !hires_map->Cancelled() && hires_map->CurrentPlanet() == m_planet)
	{
		m_map = hires_map;
		hasHires = true;
	}

	viewDist = distance;

	//std::thread new_thread(&Face::MakeFace, this, device, context);
	//new_thread.detach();

	MakeFace(device, context);

	return true;
}

void Face::MakeFace(ID3D11Device* device, ID3D11DeviceContext* context)
{
	int divide = m_divide;
	int recursion = m_recursion;
	float size = m_size;
	XMFLOAT3 offset = m_offset;
	float distance = m_distance;
	int faceDir = m_faceDir;
	Transform transform = m_transform;

	float temperature = 400;

	bool smooth = true;

	//create the face buffers
	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	std::vector<int> modifyList;
	std::vector<XMFLOAT3> posList;

	std::list<Triangle> triList;

	//create the initial face, front facing.

	//number of squares per face- will end up being doubled
	//int divide = 10;

	//tesselate
	float tes = 1.0f / ((float)divide / 2.0f);
	tes *= size;

	int pushed = 0;

	XMFLOAT3 origin = offset;

	//find the center
	XMFLOAT3 center = origin;
	center.x += size;
	center.y += size;
	center.z = -1.0f;

	int extend = 2;

	if (!smooth)
	{
		extend = 1;
	}

	int s = divide + (extend * 2);

	//this just makes the face
	for (int x = -extend; x < divide + extend; x++)
	{
		for (int y = -extend; y < divide + extend; y++)
		{
			ModelClass::VertexType temp;

			//start at the 'origin'
			temp.position = XMFLOAT3(x * tes, y * tes, -1.0f);
			temp.position = AddFloat3(temp.position, origin);
			//vertexList.push_back(temp);
			//indexList.push_back(pushed);
			//pushed++;

			if ((pushed + 1) % (s) != 0 && floor((pushed + 1) / s) != s - 1 && smooth)
			{
				if (temp.position.x < 1.0f || temp.position.y < 1.0f)
				{
					triList.push_back(Triangle(pushed, pushed + 1, pushed + s));
					triList.push_back(Triangle(pushed + 1, pushed + s + 1, pushed + s));
				}
			}

			if (smooth)
			{
				//dealing with 'cracks'
				if (x < 0 || y < 0 || x > divide || y > divide)
				{

					modifyList.push_back(pushed);

					if (temp.position.x < -1.0f && temp.position.y < -1.0f
						|| temp.position.x > 1.0f && temp.position.y < -1.0f
						|| temp.position.x < -1.0f && temp.position.y > 1.0f
						|| temp.position.x > 1.0f && temp.position.y > 1.0f)
					{
						temp.position.z += tes * 2;
						temp.color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
					}
					else
					{
						bool doThing = false;
						float add = 0.0f;

						if (temp.position.x < -1.0f)
						{
							add += abs(-1.0f - temp.position.x);
							temp.position.x = -1.0f;
							doThing = true;
						}
						if (temp.position.y < -1.0f)
						{
							add += abs(-1.0f - temp.position.y);
							temp.position.y = -1.0f;
							doThing = true;
						}
						if (temp.position.x > 1.0f)
						{
							add += abs(temp.position.x - 1.0f);
							temp.position.x = 1.0f;
							doThing = true;
						}
						if (temp.position.y > 1.0f)
						{
							add += abs(temp.position.y - 1.0f);
							temp.position.y = 1.0f;
							doThing = true;
						}

						if (doThing)
						{
							//add /= tes;
							temp.position.z += add;
							//temp.color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
						}
					}
				}
				vertexList.push_back(temp);
				if (x >= -1 && y >= -1 && x < divide + 1 && y < divide + 1)
				{
					if ((pushed + 1) % (s) != 0 && floor((pushed + 1) / s) != s - 1)
					{
						indexList.push_back(pushed);
						indexList.push_back(pushed + 1);
						indexList.push_back(pushed + s);

						indexList.push_back(pushed + 1);
						indexList.push_back(pushed + s + 1);
						indexList.push_back(pushed + s);

						//triList.push_back(Triangle(pushed, pushed + 1, pushed + s));
						//triList.push_back(Triangle(pushed + 1, pushed + s + 1, pushed + s));
					}
				}

				pushed++;
			}

			else
			{
				vertexList.push_back(temp);
				indexList.push_back(pushed);
				pushed++;

				//now go up
				temp.position = XMFLOAT3(x * tes, y * tes + tes, -1.0f);
				temp.position = AddFloat3(temp.position, origin);
				vertexList.push_back(temp);
				indexList.push_back(pushed);
				pushed++;

				//and bottom right
				temp.position = XMFLOAT3(x * tes + tes, y * tes, -1.0f);
				temp.position = AddFloat3(temp.position, origin);
				vertexList.push_back(temp);
				indexList.push_back(pushed);
				pushed++;

				//second triangle, start up
				temp.position = XMFLOAT3(x * tes, y * tes + tes, -1.0f);
				temp.position = AddFloat3(temp.position, origin);
				vertexList.push_back(temp);
				indexList.push_back(pushed);
				pushed++;

				//up right
				temp.position = XMFLOAT3(x * tes + tes, y * tes + tes, -1.0f);
				temp.position = AddFloat3(temp.position, origin);
				vertexList.push_back(temp);
				indexList.push_back(pushed);
				pushed++;

				//and bottom right
				temp.position = XMFLOAT3(x * tes + tes, y * tes, -1.0f);
				temp.position = AddFloat3(temp.position, origin);
				vertexList.push_back(temp);
				indexList.push_back(pushed);
				pushed++;
			}
		}
	}

	//now rotate the entire face
	XMMATRIX cubeRot = XMMatrixRotationY(((90.0f * (float)faceDir)) * 0.0174532925f);
	if (faceDir > 3)
	{
		cubeRot = XMMatrixRotationX((90.0f + (180.0f * (float)(faceDir - 4))) * 0.0174532925f);
	}

	if (faceDir >= 0)
	{
		for (auto it = vertexList.begin(); it != vertexList.end(); it++)
		{
			XMFLOAT3 tempCoord = it->position;
			//tempCoord = TakeFloat3(tempCoord, origin);

			//reading from heightmap
			//first, convert coordinates to the heightmap coordinates

			int h_res = m_map->GetHeightMapRes();
			//int n_res = m_map->GetNormalMapRes();

			float xCoordFloat = ((min(max(tempCoord.x, -1.0f), 1.0f) + 1.0f) / 2.0f) * (float)(h_res - 1);
			float yCoordFloat = ((min(max(tempCoord.y, -1.0f), 1.0f) + 1.0f) / 2.0f) * (float)(h_res - 1);
			float value = m_map->GetHeightMapValueFloat(faceDir, xCoordFloat, yCoordFloat);

			//xCoordFloat = ((min(max(tempCoord.x, -1.0f), 1.0f) + 1.0f) / 2.0f) * (float)(n_res - 1);
			//yCoordFloat = ((min(max(tempCoord.y, -1.0f), 1.0f) + 1.0f) / 2.0f) * (float)(n_res - 1);

			//it->normal = m_map->GetNormalMapValue(faceDir, (int)xCoordFloat, (int)yCoordFloat);
			XMFLOAT3 col = m_map->GetColorMapValue(faceDir, (int)xCoordFloat, (int)yCoordFloat);
			it->color = XMFLOAT4(col.x, col.y, col.z, 0);

			XMVECTOR tempPos = XMLoadFloat3(&tempCoord);
			tempPos = XMVector3TransformCoord(tempPos, cubeRot);
			XMStoreFloat3(&tempCoord, tempPos);
			it->position = tempCoord;

			// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
			float pitch = transform.rot.x * 0.0174532925f;
			float yaw = transform.rot.y * 0.0174532925f;
			float roll = transform.rot.z * 0.0174532925f;

			tempCoord = origin;
			origin = XMFLOAT3(-1.0f, -1.0f, 0.0f);

			// Create the rotation matrix from the yaw, pitch, and roll values.
			XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

			float newValue = (float)value;

			//now move that between 1 and 0.15
			value *= 30.0f;
			value -= 6.5f;
			value /= 3.5f;

			float tempScale = 1.0f - (value / 2.5f);
			float newTemp = temperature * tempScale;

			tempPos = XMLoadFloat3(&it->position);
			tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
			//normalize
			tempPos = XMVector3Normalize(tempPos);
			XMStoreFloat3(&it->position, tempPos);
			it->position = MultFloat3(it->position, XMFLOAT3((float)newValue, (float)newValue, (float)newValue));
			it->position = MultFloat3(it->position, transform.scale);
			it->position = AddFloat3(it->position, transform.pos);
			it->position = TakeFloat3(it->position, origin);
			//it->color = XMFLOAT4(1.0f - value, 0, value, 1.0f);
			//it->color = XMFLOAT4((0.8f - value * 0.8f), (value * 0.7f), 0.5f, 1.0f);
			//it->color = XMFLOAT4((temperature * tempScale) / 3000.0f, 0.0f, 1.0f - (temperature * tempScale) / 3000.0f, 1.0f);

			origin = tempCoord;
		}

		ModelClass::VertexType temp;
		temp.position = center;
		XMVECTOR tempPos = XMLoadFloat3(&temp.position);
		tempPos = XMVector3TransformCoord(tempPos, cubeRot);
		XMStoreFloat3(&temp.position, tempPos);
		center = temp.position;
	}

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	float pitch = transform.rot.x * 0.0174532925f;
	float yaw = transform.rot.y * 0.0174532925f;
	float roll = transform.rot.z * 0.0174532925f;

	origin = XMFLOAT3(-1.0f, -1.0f, 0.0f);

	// Create the rotation matrix from the yaw, pitch, and roll values.
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	module::Perlin myModule;
	myModule.SetOctaveCount(10);

	float pScale = 0.75f;
	XMFLOAT3 perlinScale = XMFLOAT3(pScale, pScale, pScale);
	float waterHeight = 0.30f;

	/*for (auto it = vertexList.begin(); it != vertexList.end(); it++)
	{
		//get the coordinates for the perlin noise
		XMFLOAT3 tempCoord = it->position;
		//tempCoord = MultFloat3(tempCoord, transform.scale);
		tempCoord = AddFloat3(tempCoord, transform.perlin);
		tempCoord = TakeFloat3(tempCoord, origin);

		tempCoord = MultFloat3(tempCoord, perlinScale);

		//get a value between 1 and 0
		double value = myModule.GetValue(tempCoord.x, tempCoord.y, tempCoord.z);
		value += 2.0f;
		value /= 3.0f;

		float newValue = (float)value;

		//now move that between 1 and 0.15
		newValue *= 3.5f;
		newValue += 6.5f;
		newValue /= 30.0f;

		float tempScale = 1.0f - (value / 2.5f);
		float newTemp = temperature * tempScale;

		float waterColor = 0.0f;
		float oldValue = newValue;
		bool water = false;
		bool lava = false;
		if (newValue < waterHeight && temperature < 373.2f)
		{
			waterColor = abs(newValue - waterHeight);
			waterColor /= waterHeight;
			waterColor = waterHeight - waterColor;
			waterColor *= 1.25f;
			newValue = waterHeight;
			water = true;
		}
		if (newTemp > 1000.0f)
		{
			lava = true;
			waterColor = abs(newValue - waterHeight);
			waterColor /= waterHeight;
			waterColor = waterHeight - waterColor;
			waterColor += 0.5f;
			waterColor *= 1.0f;
			newValue *= 0.95f;
		}

		XMVECTOR tempPos = XMLoadFloat3(&it->position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		//normalize
		tempPos = XMVector3Normalize(tempPos);
		XMStoreFloat3(&it->position, tempPos);
		//it->position = MultFloat3(it->position, XMFLOAT3((float)newValue, (float)newValue, (float)newValue));
		//it->position = MultFloat3(it->position, transform.scale);
		//it->position = AddFloat3(it->position, transform.pos);
		//it->position = TakeFloat3(it->position, origin);
		//it->color = XMFLOAT4(1.0f - value, 0, value, 1.0f);
		//it->color = XMFLOAT4((0.8f - value * 0.8f), (value * 0.7f), 0.5f, 1.0f);
		//it->color = XMFLOAT4((temperature * tempScale) / 3000.0f, 0.0f, 1.0f - (temperature * tempScale) / 3000.0f, 1.0f);
		if (water)
		{
			//it->color.x *= waterColor;
			//it->color.y *= waterColor;
			//it->color.z *= waterColor;
			//it->color.z += 0.25 * (waterColor - waterHeight);
		}
		if (lava)
		{
			//it->color.x *= waterColor;
			//it->color.y *= waterColor;
			//it->color.z *= waterColor * 0.25f;
			//it->color.x += 0.5 * (waterColor - waterHeight);
			//it->color.y += 0.25 * (waterColor - waterHeight);

			//it->color = XMFLOAT4(1.0f, 0.25f, 0.0f, 1.0f);
		}
	}*/

	//center

	//get the coordinates for the perlin noise
	XMFLOAT3 tempCoord = center;
	//tempCoord = MultFloat3(tempCoord, transform.scale);
	tempCoord = AddFloat3(tempCoord, transform.perlin);
	tempCoord = TakeFloat3(tempCoord, origin);

	tempCoord = MultFloat3(tempCoord, perlinScale);

	//get a value between 1 and 0
	double value = myModule.GetValue(tempCoord.x, tempCoord.y, tempCoord.z);
	value += 2.0f;
	value /= 3.0f;

	//now move that between 1 and 0.15
	value *= 3.5f;
	value += 6.5f;
	value /= 30.0f;

	if (value < waterHeight)
	{
		//value = waterHeight;
	}

	XMVECTOR tempPos = XMLoadFloat3(&center);
	tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
	//normalize
	tempPos = XMVector3Normalize(tempPos);
	XMStoreFloat3(&center, tempPos);
	center = MultFloat3(center, XMFLOAT3((float)value, (float)value, (float)value));
	center = MultFloat3(center, transform.scale);
	center = AddFloat3(center, transform.pos);
	center = TakeFloat3(center, origin);
	
	if (!smooth)
	{
		for (int i = 0; i < indexList.size(); i += 3)
		{
			XMFLOAT3 normal = ComputeNormal(vertexList[indexList[i]].position,
				vertexList[indexList[i + 1]].position, vertexList[indexList[i + 2]].position);

			vertexList[indexList[i]].normal = normal;
			vertexList[indexList[i + 1]].normal = normal;
			vertexList[indexList[i + 2]].normal = normal;
		}
	}

	else
	{
		for each(Triangle tri in triList)
		{
			XMFLOAT3 normal = ComputeNormal(vertexList[tri.v1].position,
				vertexList[tri.v2].position, vertexList[tri.v3].position);

			vertexList[tri.v1].normal = normal;
			vertexList[tri.v2].normal = normal;
			vertexList[tri.v3].normal = normal;
		}
	}

	//now the normals are calculated, the extended faces can be pushed under
	for each(int vertex in modifyList)
	{
		vertexList[vertex].position = AddFloat3(vertexList[vertex].position, origin);
		vertexList[vertex].position = TakeFloat3(vertexList[vertex].position, transform.pos);
		vertexList[vertex].position = MultFloat3(vertexList[vertex].position, XMFLOAT3(0.9f, 0.9f, 0.9f));
		vertexList[vertex].position = AddFloat3(vertexList[vertex].position, transform.pos);
		vertexList[vertex].position = TakeFloat3(vertexList[vertex].position, origin);
		//vertexList[vertex].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	TexParam temp;
	temp.type = GenType::NONE;

	m_model = new ModelClass();
	m_model->Initialize(device, context, vertexList, indexList, "", temp);

	m_pos = center;

	origin = offset;

	if (recursion)
	{
		for (int i = 0; i < 4; i++)
		{
			m_children[i] = new Face();
			m_children[i]->SetParent(this);
		}
	}

	m_map = lowres_map;

	created = true;
}

void Face::CreateChildren(ID3D11Device* device, ID3D11DeviceContext* context)
{
	if (!created) return;

	XMFLOAT3 origin = m_offset;
	if (m_recursion)
	{
		for (int i = 0; i < 4; i++)
		{
			if (!m_children[i]->Creating())
			{
				m_offset = origin;
				m_offset.x += (i % 2) * m_size;
				m_offset.y += (i > 1) * m_size;
				m_children[i]->Initialize(m_transform, m_recursion - 1, m_faceDir, m_distance * 0.5f, device, context, m_offset, m_size * 0.5f, m_divide + 1,
					m_map, hires_map, m_planet);
			}
		}
	}
	m_offset = origin;
}

std::list<ModelClass*> Face::GetModels(XMFLOAT3 camPos, ID3D11Device* device, ID3D11DeviceContext* context)
{
	XMVECTOR mPos = XMLoadFloat3(&m_pos);
	XMVECTOR cPos = XMLoadFloat3(&camPos);
	float distance;
	mPos -= cPos;
	mPos = XMVector3Length(mPos);
	XMStoreFloat(&distance, mPos);

	std::list<ModelClass*> result;

	if (!created) return result;

	if (distance > viewDist || !m_recursion)
	{
		if(created)
		{
			result.push_back(m_model);

			for (int i = 0; i < 4; i++)
			{
				if (m_children[i] != NULL)
				{
					m_children[i]->Shutdown();
					//delete m_children[i];
				}
			}
		}
	}
	else
	{

		CreateChildren(device, context);
		bool returnSelf = false;

		for (int i = 0; i < 4; i++)
		{
			if (m_children[i]->Created())
			{
				if (distance > m_children[i]->viewDist || !m_children[i]->m_recursion)
				{
					if (!m_children[i]->hasHires && hires_map->IsBuilt() && !hires_map->Cancelled() && hires_map->CurrentPlanet() == m_planet)
					{
						m_children[i]->Rebuild(device, context);
						returnSelf = true;
					}
				}
			}
			else if (!m_children[i]->Created())
			{
				returnSelf = true;
			}
		}

		if (returnSelf)
		{
			result.push_back(m_model);
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				std::list<ModelClass*> tempList = m_children[i]->GetModels(camPos, device, context);
				for each(ModelClass* model in tempList)
				{
					result.push_back(model);
				}
			}
		}
	}

	return result;
}

int Face::GetIndexCount(float distance)
{
	int result = 0;
	if (distance > viewDist || m_children[0] == NULL)
	{
		result += m_model->GetIndexCount();
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			result += m_children[i]->GetIndexCount(distance);
		}
	}

	return result;
}

XMFLOAT3 Face::GetPos()
{
	return m_pos;
}

void Face::Shutdown()
{
	for (int i = 0; i < 4; i++)
	{
		if (m_children[i] != NULL)
		{
			m_children[i]->Shutdown();
			delete m_children[i];
			m_children[i] = 0;
		}
	}

	if (m_model != NULL)
	{
		m_model->Shutdown();
		delete m_model;
		m_model = 0;
	}

	created = false;
	creating = false;

	//delete lowres_map;
	//delete hires_map;
	//delete m_map;
	//delete m_parent;
}

bool Face::Created()
{
	return created;
}

bool Face::Creating()
{
	return creating;
}

void Face::Rebuild(ID3D11Device* device, ID3D11DeviceContext* context)
{
	Shutdown();

	Initialize(m_transform, m_recursion, m_faceDir, m_distance, device, context, m_offset, m_size, m_divide, m_map, hires_map, m_planet);
}

bool Face::IsHires()
{
	return hasHires;
}

void Face::SetParent(Face* set)
{
	m_parent = set;
}