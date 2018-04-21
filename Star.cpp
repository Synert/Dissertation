#include "Star.h"

Star::Star()
{

}

Star::~Star()
{

}

void Star::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, StarParam param)
{
	m_param = param;

	if (param.temperature < 3500.0f)
	{
		m_color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (param.temperature < 5000.0f)
	{
		m_color = XMFLOAT4(0.8f, 0.5f, 0.0f, 1.0f);
	}
	else if (param.temperature < 6000.0f)
	{
		m_color = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		m_color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	}

	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	std::list<ModelClass::VertexType> cubeFace;

	//create the initial face, front facing.

	

	//number of squares per face- will end up being doubled
	int divide = 12;

	//tesselate
	float tes = 1.0f / ((float)divide / 2.0f);

	XMFLOAT3 origin = XMFLOAT3(-1.0f, -1.0f, 0.0f);

	int pushed = 0;

	for (int x = 0; x < divide; x++)
	{
		for (int y = 0; y < divide; y++)
		{
			ModelClass::VertexType temp;
			temp.color = XMFLOAT4(0.0f, (x * tes) / 2.0f, (y * tes) / 2.0f, 1.0f);

			//start at the 'origin'
			temp.position = XMFLOAT3(x * tes, y * tes, -1.0f);
			temp.position = Maths::AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//now go up
			temp.position = XMFLOAT3(x * tes, y * tes + tes, -1.0f);
			temp.position = Maths::AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//and bottom right
			temp.position = XMFLOAT3(x * tes + tes, y * tes, -1.0f);
			temp.position = Maths::AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//second face, start up
			temp.position = XMFLOAT3(x * tes, y * tes + tes, -1.0f);
			temp.position = Maths::AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//up right
			temp.position = XMFLOAT3(x * tes + tes, y * tes + tes, -1.0f);
			temp.position = Maths::AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//and bottom right
			temp.position = XMFLOAT3(x * tes + tes, y * tes, -1.0f);
			temp.position = Maths::AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		XMMATRIX cubeRot = XMMatrixRotationY((90.0f + (90.0f * (float)i)) * 0.0174532925f);
		for (auto it = cubeFace.begin(); it != cubeFace.end(); it++)
		{
			ModelClass::VertexType temp;
			temp = *it;
			XMVECTOR tempPos = XMLoadFloat3(&temp.position);
			tempPos = XMVector3TransformCoord(tempPos, cubeRot);
			XMStoreFloat3(&temp.position, tempPos);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		XMMATRIX cubeRot = XMMatrixRotationX((90.0f + (180.0f * (float)i)) * 0.0174532925f);
		for (auto it = cubeFace.begin(); it != cubeFace.end(); it++)
		{
			ModelClass::VertexType temp;
			temp = *it;
			XMVECTOR tempPos = XMLoadFloat3(&temp.position);
			tempPos = XMVector3TransformCoord(tempPos, cubeRot);
			XMStoreFloat3(&temp.position, tempPos);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;
		}
	}


	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	for (auto it = vertexList.begin(); it != vertexList.end(); it++)
	{
		XMVECTOR tempPos = XMLoadFloat3(&it->position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		//normalize
		tempPos = Maths::XMVector3NormalizeRobust(tempPos);
		XMStoreFloat3(&it->position, tempPos);
		it->position = Maths::ScalarFloat3(it->position, (param.radius / 30000.0f));
		it->position = Maths::AddFloat3(it->position, param.pos);
		it->position = Maths::TakeFloat3(it->position, origin);
		it->color = m_color;
	}

	for (int i = 0; i < indexList.size(); i += 3)
	{
		XMFLOAT3 normal = Maths::ComputeNormal(vertexList[indexList[i]].position,
			vertexList[indexList[i + 1]].position, vertexList[indexList[i + 2]].position);

		vertexList[indexList[i]].normal = normal;
		vertexList[indexList[i + 1]].normal = normal;
		vertexList[indexList[i + 2]].normal = normal;
	}

	ModelClass *newModel = new ModelClass();
	TexParam temp;
	temp.type = GenType::NONE;
	m_model = new ModelClass;
	m_model->Initialize(device, context, vertexList, indexList, "", temp);
}

StarParam Star::GetParam()
{
	return m_param;
}

ModelClass* Star::GetModel()
{
	return m_model;
}

XMFLOAT4 Star::GetColor()
{
	XMFLOAT4 retColor = m_color;
	retColor.x += 2.0f;
	retColor.y += 2.0f;
	retColor.z += 2.0f;
	retColor = Maths::ScalarFloat4(retColor, 0.33f);
	return retColor;
}
