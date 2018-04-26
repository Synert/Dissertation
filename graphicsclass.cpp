#include "graphicsclass.h"
#include <stdlib.h>
#include <time.h>
#include <AntTweakBar.h>

using namespace Maths;

GraphicsClass::GraphicsClass()
{
	m_Direct3D = 0;
	m_Model = 0;
	m_ColorShader = 0;
	m_TextureShader = 0;

	new_transform.scale = one;
	new_transform.rot = zero;
	new_transform.pos = zero;
	new_transform.color = COL_WHITE;

	trunk_param.rings = 6;
	trunk_param.segments = 6;
	trunk_param.startWidth = 1.0f;
	trunk_param.endWidth = 0.2f;
	trunk_param.segHeight = 1.0f;
	trunk_param.upperBound = 3.0f;
	trunk_param.lowerBound = -3.0f;
	trunk_param.recursion = 3;

	leaf_param.rings = 5;
	leaf_param.segments = 8;
	leaf_param.startWidth = 0.0f;
	leaf_param.segHeight = 1.0f;
	leaf_param.lowerBound = 0.0f;
	leaf_param.upperBound = 5.0f;
	leaf_param.lowerBoundX = 0.25f;
	leaf_param.lowerBoundZ = 0.25f;
	leaf_param.upperBoundX = 0.5f;
	leaf_param.upperBoundY = 0.5f;
	leaf_param.upperBoundZ = 0.5f;

	tex_param.type = GenType::NONE;
	leaf_tex_param.type = GenType::LEAF;
	trunk_tex_param.type = GenType::BARK;
	rose_tex_param.type = GenType::ROSE;

	leaf_tex_param.baseCol = COL_LEAFGREEN;
	leaf_tex_param.baseVar = XMFLOAT4(5, 15, 12, 0);
	leaf_tex_param.altCol = COL_DARKTURQUOISE;
	leaf_tex_param.altVar = XMFLOAT4(20, 20, 20, 0);
	leaf_tex_param.thickness = 6;

	trunk_tex_param.baseCol = COL_TRUNKBROWN;
	trunk_tex_param.baseVar = XMFLOAT4(5, 15, 12, 0);
	trunk_tex_param.altCol = COL_LEAFGREEN;
	trunk_tex_param.altVar = XMFLOAT4(20, 20, 20, 0);
	trunk_tex_param.thickness = 2;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd, CameraClass* cam)
{
	bool result;

	m_Camera = cam;

	// Create the Direct3D object.
	m_Direct3D = new D3DClass;

	// Initialize the Direct3D object.
	m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);

	m_TextureShader = new TextureShaderClass;
	m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);

	m_ColorShader = new ColorShaderClass;
	m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd);

	m_light = new Light;
	m_light->SetLightAmbient(XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));
	m_light->SetLightDiffuse(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_light->SetLightDirection(XMFLOAT3(1.0f, -0.5f, 0.0f));

	TwInit(TW_DIRECT3D11, m_Direct3D->GetDevice()); // for Direct3D 11
	TwWindowSize(m_Direct3D->GetWindowWidth(), m_Direct3D->GetWindowHeight());

	srand(time(NULL));

	m_hiresMap = new Mapping();

	RemakeSystem();

	return true;
}

//static bool deleteAll(ModelClass * theElement) { delete theElement; return true; }

void GraphicsClass::Shutdown()
{

	// Release the color shader object.
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader;
		m_ColorShader = 0;
	}

	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	// Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}

	//delete the models
	m_objects.clear();

	return;
}


bool GraphicsClass::Frame(TwBar* bar)
{
	bool result;


	// Render the graphics scene.
	result = Render(bar);
	if (!result)
	{
		return false;
	}

	return true;
}


bool GraphicsClass::Render(TwBar* bar)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	if (remaking)
	{
		RemakeSystem();
		return true;
	}
	if (remakingPlanet)
	{
		RemakePlanet();
	}
	if (remakingStar)
	{
		RemakeStar();
	}

	// Clear the buffers to begin the scene.

	//get the sky colour of the closest planet

	XMFLOAT4 tempCol = COL_BLACK;
	Planet* closest = NULL;
	int planetID = 0;
	float distance = 9999999999999.0f;

	for (int i = 0; i < m_planets.size(); i++)
	{
		if (m_planets[i]->Built())
		{
			XMVECTOR _pos = XMLoadFloat3(&m_Camera->GetPosition());
			XMVECTOR _spos = XMLoadFloat3(&m_planets[i]->GetPosition());
			float p_distance;
			_pos -= _spos;
			_pos = XMVector3Length(_pos);
			XMStoreFloat(&p_distance, _pos);
			if (m_planets[i]->DrawSky(m_Camera->GetPosition()))
			{
				tempCol = m_planets[i]->GetSky();
			}
			if (p_distance < distance)
			{
				distance = p_distance;
				closest = m_planets[i];
				planetID = i;
			}
		}
	}


	if (closest != NULL)
	{
		if (closest != builtPlanet)
		{
			m_hiresMap->Cancel();
			m_hiresMap->Shutdown();
			m_hiresMap->SetPlanet(NULL);

			planetParam.m_waterHeight = closest->GetWaterHeight();
			planetParam.p_size = closest->GetSize();
			planetParam.p_sky = closest->GetUnscaledSky();
			planetParam.p_position = closest->GetPosition();
			planetParam.p_seed = closest->GetPerlin();
			planetParam.p_temperature = closest->GetTemperature();

			planetParam.m_seed = closest->GetMapPerlin();
			planetParam.p_flat = closest->GetFlat();

		}
		if (!m_hiresMap->IsBuilding())
		{
			//create the hires map here
			m_hiresMap->Setup(1024, closest->GetTemperature(), closest->GetPerlin(), closest->GetMapPerlin(), closest->GetWaterHeight(), closest->GetFlat());
			m_hiresMap->SetPlanet(closest);
			builtPlanet = closest;
		}
	}
	else
	{
		m_hiresMap->Shutdown();
	}

	m_Direct3D->BeginScene(tempCol);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	/*XMFLOAT3 oldPos = m_light->GetLightDirection();
	XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(0.0f * 0.0174532925f, 1.0f * 0.0174532925f, 0.0f * 0.0174532925f);

	XMVECTOR tempPos = XMLoadFloat3(&oldPos);
	tempPos = XMVector3TransformCoord(tempPos, rotMat);
	XMStoreFloat3(&oldPos, tempPos);

	m_light->SetLightDirection(oldPos);*/


	for each (ModelClass* model in m_objects)
	{
		model->Render(m_Direct3D->GetDeviceContext());
		//m_ColorShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

		if (model->GetTexture() != NULL)
		{
			m_TextureShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, model->GetTexture(), m_light);
		}
		else
		{
			m_ColorShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_light);
		}
	}

	triCount = 0;

	m_light->SetLightAmbient(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_star->GetModel()->Render(m_Direct3D->GetDeviceContext());
	m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_star->GetModel()->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_light);
	m_light->SetLightAmbient(XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f));

	for each(Planet* planet in m_planets)
	{
		std::list<ModelClass*> faceModels = planet->GetModels(m_Camera->GetPosition(), m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_hiresMap);
		int count = 0;
		XMVECTOR vecPos = XMLoadFloat3(&planet->GetPosition());
		vecPos = Maths::XMVector3NormalizeRobust(vecPos);
		XMFLOAT3 newPos;
		XMStoreFloat3(&newPos, vecPos);
		m_light->SetLightDirection(newPos);

		for each(ModelClass* model in faceModels)
		{
			model->Render(m_Direct3D->GetDeviceContext());
			//m_ColorShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);
			triCount += model->GetIndexCount() / 3;

			if (model->GetTexture() != NULL)
			{
				m_TextureShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, model->GetTexture(), m_light);
			}
			else
			{
				m_ColorShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_light);
			}

			//count++;
		}
	}

	//m_ColorShader->Render(m_Direct3D->GetDeviceContext(), m_face->GetIndexCount(length), worldMatrix, viewMatrix, projectionMatrix, m_light);


	TwRefreshBar(bar);
	TwDraw();

	// Render the model using the color shader.
	//result = m_ColorShader->Render(m_Direct3D->GetDeviceContext(), indexCount, worldMatrix, viewMatrix, projectionMatrix);
	//if (!result)
	//{
		//return false;
	//}

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

void GraphicsClass::RenderModel(ModelClass* model)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	model->Render(m_Direct3D->GetDeviceContext());
	//m_ColorShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix);

	if (model->GetTexture() != NULL)
	{
		m_TextureShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, model->GetTexture(), m_light);
	}
	else
	{
		m_ColorShader->Render(m_Direct3D->GetDeviceContext(), model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_light);
	}
}

float GraphicsClass::GetSpeed(XMFLOAT3 position)
{
	//first, get the closest planet

	float lowest = 9999999999999;
	Planet* closest = NULL;

	for each(Planet* planet in m_planets)
	{
		XMVECTOR mPos = XMLoadFloat3(&planet->GetPosition());
		XMVECTOR cPos = XMLoadFloat3(&position);
		float distance;
		mPos -= cPos;
		mPos = XMVector3Length(mPos);
		XMStoreFloat(&distance, mPos);

		if (distance < lowest)
		{
			lowest = distance;
			closest = planet;
		}
	}

	if (closest == NULL) return 0.5f;

	float base = 0.5f;
	lowest /= (closest->GetSize() * base * 10.0f);
	float speed = base * lowest;
	if (speed > base) speed = base;

	return speed;
}

void GraphicsClass::CreateCube(Transform transform)
{
	int m_vertexCount = 8;
	int m_indexCount = 36;

	ModelClass::VertexType* vertices;
	unsigned long* indices;

	vertices = new ModelClass::VertexType[m_vertexCount];
	indices = new unsigned long[m_indexCount];

	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, -1.0f);  //Front, bottom left
	vertices[0].color = transform.color;
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(-1.0f, 1.0f, -1.0f);  //Front, top left
	vertices[1].color = transform.color;
	vertices[1].texture = XMFLOAT2(0.0f, 1.0f);

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, -1.0f);  //Front, bottom right
	vertices[2].color = transform.color;
	vertices[2].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[3].position = XMFLOAT3(1.0f, 1.0f, -1.0f);  //Front, top right
	vertices[3].color = transform.color;
	vertices[3].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[4].position = XMFLOAT3(-1.0f, -1.0f, 1.0f);  //Back, bottom left
	vertices[4].color = transform.color;
	vertices[4].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[5].position = XMFLOAT3(-1.0f, 1.0f, 1.0f);  //Back, top left
	vertices[5].color = transform.color;
	vertices[5].texture = XMFLOAT2(0.0f, 1.0f);

	vertices[6].position = XMFLOAT3(1.0f, -1.0f, 1.0f);  //Back, bottom right
	vertices[6].color = transform.color;
	vertices[6].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[7].position = XMFLOAT3(1.0f, 1.0f, 1.0f);  //Back, top right
	vertices[7].color = transform.color;
	vertices[7].texture = XMFLOAT2(1.0f, 1.0f);

	// Load the index array with data.
	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top middle.
	indices[2] = 2;  // Bottom right.

	indices[3] = 3;  // Bottom left.
	indices[4] = 2;  // Top middle.
	indices[5] = 1;  // Bottom right.

	indices[6] = 2;  // Bottom left.
	indices[7] = 3;  // Top middle.
	indices[8] = 6;  // Bottom right.

	indices[9] = 6;  // Bottom left.
	indices[10] = 3;  // Top middle.
	indices[11] = 7;  // Bottom right.

	indices[12] = 0;  // Bottom left.
	indices[13] = 4;  // Top middle.
	indices[14] = 1;  // Bottom right.

	indices[15] = 4;  // Bottom left.
	indices[16] = 5;  // Top middle.
	indices[17] = 1;  // Bottom right.

	indices[18] = 4;  // Bottom left.
	indices[19] = 6;  // Top middle.
	indices[20] = 5;  // Bottom right.

	indices[21] = 7;  // Bottom left.
	indices[22] = 5;  // Top middle.
	indices[23] = 6;  // Bottom right.

	indices[24] = 1;  // Bottom left.
	indices[25] = 5;  // Top middle.
	indices[26] = 3;  // Bottom right.

	indices[27] = 5;  // Bottom left.
	indices[28] = 7;  // Top middle.
	indices[29] = 3;  // Bottom right.

	indices[30] = 0;  // Bottom left.
	indices[31] = 2;  // Top middle.
	indices[32] = 4;  // Bottom right.

	indices[33] = 4;  // Bottom left.
	indices[34] = 2;  // Top middle.
	indices[35] = 6;  // Bottom right.*/

	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	float pitch = transform.rot.x * 0.0174532925f;
	float yaw = transform.rot.y * 0.0174532925f;
	float roll = transform.rot.z * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	for (int i = 0; i < m_vertexCount; i++)
	{
		XMVECTOR tempPos = XMLoadFloat3(&vertices[i].position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		XMStoreFloat3(&vertices[i].position, tempPos);
		vertices[i].position = MultFloat3(vertices[i].position, transform.scale);
		vertices[i].position = AddFloat3(vertices[i].position, transform.pos);
		vertexList.push_back(vertices[i]);
	}

	for (int i = 0; i < m_indexCount; i++)
	{
		indexList.push_back(indices[i]);
	}

	for (int i = 0; i < indexList.size(); i += 3)
	{
		XMFLOAT3 normal = ComputeNormal(vertexList[indexList[i]].position,
			vertexList[indexList[i + 1]].position, vertexList[indexList[i + 2]].position);

		vertexList[indexList[i]].normal = normal;
		vertexList[indexList[i + 1]].normal = normal;
		vertexList[indexList[i + 2]].normal = normal;
	}

	ModelClass *newModel = new ModelClass();
	newModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), vertexList, indexList, "", tex_param);
	m_objects.push_back(newModel);
}

void GraphicsClass::CreateSphere(Transform transform, int divide)
{
	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	std::list<ModelClass::VertexType> cubeFace;

	//create the initial face, front facing.

	//number of squares per face- will end up being doubled
	//int divide = 2;

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
			temp.position = AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//now go up
			temp.position = XMFLOAT3(x * tes, y * tes + tes, -1.0f);
			temp.position = AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//and bottom right
			temp.position = XMFLOAT3(x * tes + tes, y * tes, -1.0f);
			temp.position = AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//second face, start up
			temp.position = XMFLOAT3(x * tes, y * tes + tes, -1.0f);
			temp.position = AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//up right
			temp.position = XMFLOAT3(x * tes + tes, y * tes + tes, -1.0f);
			temp.position = AddFloat3(temp.position, origin);
			cubeFace.push_back(temp);
			vertexList.push_back(temp);
			indexList.push_back(pushed);
			pushed++;

			//and bottom right
			temp.position = XMFLOAT3(x * tes + tes, y * tes, -1.0f);
			temp.position = AddFloat3(temp.position, origin);
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
	float pitch = transform.rot.x * 0.0174532925f;
	float yaw = transform.rot.y * 0.0174532925f;
	float roll = transform.rot.z * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	for (auto it = vertexList.begin(); it != vertexList.end(); it++)
	{
		XMVECTOR tempPos = XMLoadFloat3(&it->position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		//normalize
		tempPos = XMVector3NormalizeRobust(tempPos);
		XMStoreFloat3(&it->position, tempPos);
		it->position = MultFloat3(it->position, transform.scale);
		it->position = AddFloat3(it->position, transform.pos);
		it->position = TakeFloat3(it->position, origin);
	}

	for (int i = 0; i < indexList.size(); i += 3)
	{
		XMFLOAT3 normal = ComputeNormal(vertexList[indexList[i]].position,
			vertexList[indexList[i + 1]].position, vertexList[indexList[i + 2]].position);

		vertexList[indexList[i]].normal = normal;
		vertexList[indexList[i + 1]].normal = normal;
		vertexList[indexList[i + 2]].normal = normal;
	}

	ModelClass *newModel = new ModelClass();
	newModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), vertexList, indexList, "", tex_param);
	m_objects.push_back(newModel);
}

void GraphicsClass::CreateTriangle(Transform transform)
{
	int m_vertexCount = 3;
	int m_indexCount = 6;

	ModelClass::VertexType* vertices;
	unsigned long* indices;

	vertices = new ModelClass::VertexType[m_vertexCount];
	indices = new unsigned long[m_indexCount];

	// Load the vertex array with data.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, -1.0f);  //Front, bottom left
	vertices[0].color = transform.color;
	vertices[0].texture = XMFLOAT2(0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(-1.0f, 1.0f, -1.0f);  //Front, top left
	vertices[1].color = transform.color;
	vertices[1].texture = XMFLOAT2(0.5f, 0.0f);

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, -1.0f);  //Front, bottom right
	vertices[2].color = transform.color;
	vertices[2].texture = XMFLOAT2(1.0f, 1.0f);

	// Load the index array with data.
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 1;

	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	float pitch = transform.rot.x * 0.0174532925f;
	float yaw = transform.rot.y * 0.0174532925f;
	float roll = transform.rot.z * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	for (int i = 0; i < m_vertexCount; i++)
	{
		XMVECTOR tempPos = XMLoadFloat3(&vertices[i].position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		XMStoreFloat3(&vertices[i].position, tempPos);
		vertices[i].position = MultFloat3(vertices[i].position, transform.scale);
		vertices[i].position = AddFloat3(vertices[i].position, transform.pos);
		vertexList.push_back(vertices[i]);
	}

	for (int i = 0; i < m_indexCount; i++)
	{
		indexList.push_back(indices[i]);
	}

	ModelClass *newModel = new ModelClass();
	newModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), vertexList, indexList, "../Procedural-ATP/leaf.tga", tex_param);
	m_objects.push_back(newModel);
}

void GraphicsClass::CreateLeaves(Transform transform, Parameters param)
{
	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	XMFLOAT3 newRot;
	newRot.x = 0.0f;
	newRot.y = 0.0f;
	newRot.z = 0.0f;

	XMFLOAT3 up;
	up.x = 0.0f;
	up.y = param.segHeight;
	up.z = 0.0f;

	XMFLOAT3 tempUp;
	tempUp = newRot;

	int leaves = param.rings;
	int segments = param.segments;

	float localScale = param.startWidth;
	ModelClass::VertexType* thisRing = new ModelClass::VertexType[leaves];
	ModelClass::VertexType* prevRing = new ModelClass::VertexType[leaves];

	float angle = (360.0f / leaves) * 0.0174532925f;

	//initial ring
	for (int i = 0; i < leaves; i++)
	{
		/*prevRing[i].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMMATRIX rotMat = XMMatrixRotationY(angle * i);
		XMVECTOR tempPos = XMLoadFloat3(&prevRing[i].position);
		tempPos = XMVector3TransformCoord(tempPos, rotMat);
		XMStoreFloat3(&prevRing[i].position, tempPos);

		prevRing[i].position = AddFloat3(prevRing[i].position, origin);*/

		prevRing[i].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		prevRing[i].texture = XMFLOAT2(0.5f, 0.0f);
		prevRing[i].color = transform.color;
	}

	int segmentsAdded = 0;

	for (int i = 0; i < segments; i++)
	{
		//random rotation
		newRot.x += RandFloat(param.lowerBound, param.upperBound);
		newRot.y += RandFloat(param.lowerBound, param.upperBound);
		newRot.z += RandFloat(param.lowerBound, param.upperBound);

		float pitch = newRot.x * 0.0174532925f;
		float yaw = newRot.y * 0.0174532925f;
		float roll = newRot.z * 0.0174532925f;

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
		XMVECTOR upVector = XMLoadFloat3(&up);

		//rotate the up vector
		upVector = XMVector3TransformCoord(upVector, rotationMatrix);

		XMStoreFloat3(&up, upVector);

		localScale = param.endWidth + ((segments - i) / segments) * (param.startWidth - param.endWidth);

		//set the new ring up
		for (int j = 0; j < leaves; j++)
		{
			thisRing[j].position = XMFLOAT3(localScale, 0.0f, 0.0f);
			thisRing[j].position = AddFloat3(up, thisRing[j].position);
			thisRing[j].position.x += RandFloat(param.lowerBoundX, param.upperBoundX) * param.segHeight;
			thisRing[j].position.y += RandFloat(param.lowerBoundY, param.upperBoundY) * param.segHeight;
			thisRing[j].position.z += RandFloat(param.lowerBoundZ, param.upperBoundZ) * param.segHeight;
			//thisRing[j].position = MultFloat3(thisRing[j].position, XMFLOAT3(scale, scale, scale));
			XMMATRIX rotMat = XMMatrixRotationY(angle * j);
			XMVECTOR tempPos = XMLoadFloat3(&thisRing[j].position);
			tempPos = XMVector3TransformCoord(tempPos, rotMat);
			tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
			XMStoreFloat3(&thisRing[j].position, tempPos);

			//thisRing[j].position = AddFloat3(up, thisRing[j].position);
			thisRing[j].position = AddFloat3(prevRing[j].position, thisRing[j].position);

			thisRing[j].color = prevRing[j].color;
			thisRing[j].color.x *= 1 + 0.75f * localScale;
			thisRing[j].color.y *= 1 + 0.75f * localScale;
			thisRing[j].color.z *= 1 + 0.75f * localScale;

			float texcoord = (float)(i) / (float)(segments - 1);

			thisRing[j].texture = XMFLOAT2(texcoord, 1.0f);

			//push the previous ring again
			vertexList.push_back(prevRing[j]);
		}

		//now push the current ring
		for (int j = 0; j < leaves; j++)
		{
			vertexList.push_back(thisRing[j]);

			prevRing[j].position = thisRing[j].position;
			prevRing[j].color = thisRing[j].color;
			prevRing[j].texture = thisRing[j].texture;

			//and set up the index list

			//self, self + ring, (self + 1) % ring
			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + leaves);
			indexList.push_back((segmentsAdded + 1) % leaves);

			//and the reverse
			//indexList.push_back(segmentsAdded);
			//indexList.push_back((segmentsAdded + 1) % leaves);
			//indexList.push_back(segmentsAdded + leaves);
			segmentsAdded++;
		}

		//skip a ring for texturing purposes
		segmentsAdded += leaves;
	}

	float pitch = transform.rot.x * 0.0174532925f;
	float yaw = transform.rot.y * 0.0174532925f;
	float roll = transform.rot.z * 0.0174532925f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	//now to duplicate the vectors
	int tempVert = vertexList.size();

	for (int i = 0; i < tempVert; i++)
	{
		vertexList.push_back(vertexList[i]);
	}

	int temp = indexList.size();

	for (int i = 0; i < temp; i+=3)
	{
		indexList.push_back(indexList[i] + tempVert);
		indexList.push_back(indexList[i + 2] + tempVert);
		indexList.push_back(indexList[i + 1] + tempVert);
	}

	for (auto it = vertexList.begin(); it != vertexList.end(); it++)
	{
		it->position = MultFloat3(it->position, transform.scale);
		XMVECTOR tempPos = XMLoadFloat3(&it->position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		XMStoreFloat3(&it->position, tempPos);
		it->position = AddFloat3(it->position, transform.pos);
	}

	for (int i = 0; i < indexList.size(); i += 3)
	{
		XMFLOAT3 normal = ComputeNormal(vertexList[indexList[i]].position,
			vertexList[indexList[i + 1]].position, vertexList[indexList[i + 2]].position);

		vertexList[indexList[i]].normal = normal;
		vertexList[indexList[i + 1]].normal = normal;
		vertexList[indexList[i + 2]].normal = normal;
	}

	ModelClass *newModel = new ModelClass();
	newModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), vertexList, indexList, "../Procedural-ATP/leaf.tga", leaf_tex_param);
	m_objects.push_back(newModel);
}

void GraphicsClass::CreateMoreLeaves(Transform transform, Parameters param)
{
	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	XMFLOAT3 newRot;
	newRot.x = 0.0f;
	newRot.y = 0.0f;
	newRot.z = 0.0f;

	int leaves = param.rings;
	int segments = param.segments;

	float localScale = 1.0f;
	ModelClass::VertexType* thisRing = new ModelClass::VertexType[leaves];
	ModelClass::VertexType* prevRing = new ModelClass::VertexType[leaves];
	ModelClass::VertexType* thisRing2 = new ModelClass::VertexType[leaves];
	ModelClass::VertexType* prevRing2 = new ModelClass::VertexType[leaves];

	float angle = (360.0f / (leaves / 4.0f)) * 0.0174532925f;

	float size = 5.0f;

	ModelClass::VertexType origin;
	origin.color = transform.color;
	origin.position = zero;

	float up = 0.2f;
	float out = 1.0f;

	//initial ring
	for (int i = 0; i < leaves; i++)
	{
		prevRing[i].position = XMFLOAT3(0.0f, up, out);

		up += 1.0f / leaves;
		out -= 1.0f / leaves;

		XMMATRIX rotMat = XMMatrixRotationY(angle * i);
		XMVECTOR tempPos = XMLoadFloat3(&prevRing[i].position);
		tempPos = XMVector3NormalizeRobust(tempPos);
		tempPos *= size;
		tempPos = XMVector3TransformCoord(tempPos, rotMat);
		XMStoreFloat3(&prevRing[i].position, tempPos);

		//prevRing[i].position = ScalarFloat3(prevRing[i].position, size);

		//prevRing[i].position = XMFLOAT3(0.0f, 0.0f, 0.0f);

		prevRing[i].color = transform.color;

		prevRing2[i] = prevRing[i];
	}

	int segmentsAdded = 0;

	for (int i = 0; i < segments; i++)
	{
		localScale = ((float)(segments - i) / (float)segments) * size;

		//set the new ring up
		for (int j = 0; j < leaves; j++)
		{
			float x = ((angle * 1.75f) / (float)segments) * (rand() % segments) / (float)segments;
			float y = ((angle * 1.0f) / (float)segments) * (rand() % segments) / (float)segments;
			float z = ((angle * 1.5f) / (float)segments) * (rand() % segments) / (float)segments;
			//x = 0.0f;
			//y = 0.0f;
			//z = 0.0f;

			XMFLOAT3 newRot = XMFLOAT3(x, y, z);

			//XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(x, y, z);
			XMMATRIX rotMat = XMMatrixRotationY(angle * j);

			XMVECTOR tempRot = XMLoadFloat3(&newRot);
			tempRot = XMVector3TransformCoord(tempRot, rotMat);
			XMStoreFloat3(&newRot, tempRot);

			rotMat = XMMatrixRotationRollPitchYaw(newRot.x, newRot.y, newRot.z);

			//XMMATRIX rotMat = XMMatrixRotationY((angle * j) * (0.5f / segments));

			thisRing[j].position = prevRing[j].position;
			thisRing[j].position.x += ((rand() % 10) - 5) / 100.0f;
			thisRing[j].position.y += ((rand() % 10) - 5) / 100.0f;
			thisRing[j].position.z += ((rand() % 10) - 5) / 100.0f;

			XMVECTOR tempPos = XMLoadFloat3(&thisRing[j].position);
			tempPos = XMVector3NormalizeRobust(tempPos);
			tempPos *= localScale;
			tempPos = XMVector3TransformCoord(tempPos, rotMat);
			XMStoreFloat3(&thisRing[j].position, tempPos);

			thisRing2[j].position = prevRing2[j].position;
			thisRing2[j].position.x -= ((rand() % 10) - 5) / 100.0f;
			thisRing2[j].position.y -= ((rand() % 10) - 5) / 100.0f;
			thisRing2[j].position.z -= ((rand() % 10) - 5) / 100.0f;

			rotMat = XMMatrixRotationRollPitchYaw(-x, -y, -z);

			tempPos = XMLoadFloat3(&thisRing2[j].position);
			tempPos = XMVector3NormalizeRobust(tempPos);
			tempPos *= localScale;
			tempPos = XMVector3TransformCoord(tempPos, rotMat);
			XMStoreFloat3(&thisRing2[j].position, tempPos);

			//thisRing[j].position = ScalarFloat3(thisRing[j].position, localScale);

			thisRing[j].color = prevRing[j].color;
			thisRing[j].color.x *= (size / localScale) * 0.2f + 0.5f;
			thisRing[j].color.y *= (size / localScale) * 0.2f + 0.5f;
			thisRing[j].color.z *= (size / localScale) * 0.2f + 0.5f;

			thisRing2[j].color = prevRing2[j].color;
			thisRing2[j].color.x *= (size / localScale) * 0.2f + 0.5f;
			thisRing2[j].color.y *= (size / localScale) * 0.2f + 0.5f;
			thisRing2[j].color.z *= (size / localScale) * 0.2f + 0.5f;

			origin.texture = XMFLOAT2(0.0f, 0.0f);
			prevRing[j].texture = XMFLOAT2(0.5f, 1.0f);
			thisRing[j].texture = XMFLOAT2(1.0f, 0.0f);

			prevRing2[j].texture = XMFLOAT2(0.5f, 1.0f);
			thisRing2[j].texture = XMFLOAT2(1.0f, 0.0f);

			vertexList.push_back(origin); //0
			vertexList.push_back(prevRing[j]); //1
			vertexList.push_back(thisRing[j]); //2

			vertexList.push_back(origin); //3
			vertexList.push_back(prevRing2[j]); //4
			vertexList.push_back(thisRing2[j]); //5

			prevRing[j].position = thisRing[j].position;
			prevRing[j].color = thisRing[j].color;

			prevRing2[j].position = thisRing2[j].position;
			prevRing2[j].color = thisRing2[j].color;

			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 1);
			indexList.push_back(segmentsAdded + 2);

			//and the reverse
			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 2);
			indexList.push_back(segmentsAdded + 1);
			segmentsAdded+=3;

			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 1);
			indexList.push_back(segmentsAdded + 2);

			//and the reverse
			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 2);
			indexList.push_back(segmentsAdded + 1);
			segmentsAdded += 3;
		}

		//skip a ring for texturing purposes
		//segmentsAdded += leaves;
	}

	float pitch = transform.rot.x * 0.0174532925f;
	float yaw = transform.rot.y * 0.0174532925f;
	float roll = transform.rot.z * 0.0174532925f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	//for texturing purposes
	float max_x = 0.0f;
	float max_y = 0.0f;
	float min_x = 0.0f;
	float min_y = 0.0f;
	float min_z = 0.0f;
	float max_z = 0.0f;

	//first, get the bounds of the model
	for (auto it = vertexList.begin(); it != vertexList.end(); it++)
	{
		if (it->position.y > max_y)
		{
			max_y = it->position.y;
		}

		if (it->position.y < min_y)
		{
			min_y = it->position.y;
		}

		if (it->position.x > max_x)
		{
			max_x = it->position.x;
		}

		if (it->position.x < min_x)
		{
			min_x = it->position.x;
		}

		if (it->position.z > max_z)
		{
			max_z = it->position.z;
		}

		if (it->position.z < min_z)
		{
			min_z = it->position.z;
		}
	}

	float total_y = max_y - min_y;
	float total_x = max_x - min_x;
	float total_z = max_z - min_z;

	min_x *= -1;
	min_z *= -1;
	min_y *= -1;

	for (int i = 0; i < indexList.size(); i += 3)
	{
		XMFLOAT3 normal = ComputeNormal(vertexList[indexList[i]].position,
			vertexList[indexList[i + 1]].position, vertexList[indexList[i + 2]].position);

		vertexList[indexList[i]].normal = normal;
		vertexList[indexList[i + 1]].normal = normal;
		vertexList[indexList[i + 2]].normal = normal;
	}

	for (auto it = vertexList.begin(); it != vertexList.end(); it++)
	{
		float tex_x = it->position.x + min_x;
		float tex_y = it->position.y + min_y;
		float tex_z = it->position.z + min_z;

		tex_x /= total_x;
		tex_y /= total_y;
		tex_z /= total_y;

		//it->texture = XMFLOAT2((tex_x + tex_z) / 2.0f, tex_y);

		it->position = MultFloat3(it->position, transform.scale);
		XMVECTOR tempPos = XMLoadFloat3(&it->position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		XMStoreFloat3(&it->position, tempPos);
		it->position = AddFloat3(it->position, transform.pos);
	}

	ModelClass *newModel = new ModelClass();
	newModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), vertexList, indexList, "../Procedural-ATP/tileleaf.tga", leaf_tex_param);
	m_objects.push_back(newModel);
}

void GraphicsClass::CreateRose(Transform transform, Parameters param)
{
	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	XMFLOAT3 newRot;
	newRot.x = 0.0f;
	newRot.y = 0.0f;
	newRot.z = 0.0f;

	int leaves = param.rings;
	int segments = param.segments;

	float localScale = 1.0f;
	ModelClass::VertexType* thisRing = new ModelClass::VertexType[leaves];
	ModelClass::VertexType* prevRing = new ModelClass::VertexType[leaves];
	ModelClass::VertexType* thisRing2 = new ModelClass::VertexType[leaves];
	ModelClass::VertexType* prevRing2 = new ModelClass::VertexType[leaves];

	float angle = (360.0f / (leaves / 3.0f)) * 0.0174532925f;

	float size = 5.0f;

	ModelClass::VertexType origin;
	origin.color = transform.color;
	origin.position = zero;

	float up = 0.25f;
	float out = 1.0f;

	//initial ring
	for (int i = 0; i < leaves; i++)
	{
		prevRing[i].position = XMFLOAT3(0.0f, up, out);

		up += 1.0f / leaves;
		out -= 1.0f / leaves;

		XMMATRIX rotMat = XMMatrixRotationY(angle * i);
		XMVECTOR tempPos = XMLoadFloat3(&prevRing[i].position);
		tempPos = XMVector3NormalizeRobust(tempPos);
		tempPos *= size;
		tempPos = XMVector3TransformCoord(tempPos, rotMat);
		XMStoreFloat3(&prevRing[i].position, tempPos);

		//prevRing[i].position = ScalarFloat3(prevRing[i].position, size);

		//prevRing[i].position = XMFLOAT3(0.0f, 0.0f, 0.0f);

		prevRing[i].color = transform.color;

		prevRing2[i] = prevRing[i];
	}

	int segmentsAdded = 0;

	for (int i = 0; i < segments; i++)
	{
		localScale = ((float)(segments - i) / (float)segments) * size;

		//set the new ring up
		for (int j = 0; j < leaves; j++)
		{
			float x = ((angle * 1.5f) / (float)segments) * (rand() % segments) / (float)segments;
			float y = ((angle * 1.0f) / (float)segments) * (rand() % segments) / (float)segments;
			float z = ((angle * 1.5f) / (float)segments) * (rand() % segments) / (float)segments;
			//XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(x, y, z);
			XMMATRIX rotMat = XMMatrixRotationY((angle * j) * (0.5f / segments));

			thisRing[j].position = prevRing[j].position;
			thisRing[j].position.x += ((rand() % 10) - 5) / 25.0f;
			thisRing[j].position.y += ((rand() % 10) - 5) / 25.0f;
			thisRing[j].position.z += ((rand() % 10) - 5) / 25.0f;

			XMVECTOR tempPos = XMLoadFloat3(&thisRing[j].position);
			tempPos = XMVector3NormalizeRobust(tempPos);
			tempPos *= localScale;
			tempPos = XMVector3TransformCoord(tempPos, rotMat);
			XMStoreFloat3(&thisRing[j].position, tempPos);

			thisRing2[j].position = prevRing2[j].position;

			rotMat = XMMatrixRotationRollPitchYaw(-x, -y, -z);

			tempPos = XMLoadFloat3(&thisRing2[j].position);
			tempPos = XMVector3NormalizeRobust(tempPos);
			tempPos *= localScale;
			tempPos = XMVector3TransformCoord(tempPos, rotMat);
			XMStoreFloat3(&thisRing2[j].position, tempPos);

			//thisRing[j].position = ScalarFloat3(thisRing[j].position, localScale);

			thisRing[j].color = prevRing[j].color;
			thisRing[j].color.x *= (size / localScale) * 0.2f + 0.5f;
			thisRing[j].color.y *= (size / localScale) * 0.2f + 0.5f;
			thisRing[j].color.z *= (size / localScale) * 0.2f + 0.5f;

			thisRing2[j].color = prevRing2[j].color;
			thisRing2[j].color.x *= (size / localScale) * 0.2f + 0.5f;
			thisRing2[j].color.y *= (size / localScale) * 0.2f + 0.5f;
			thisRing2[j].color.z *= (size / localScale) * 0.2f + 0.5f;

			vertexList.push_back(origin); //0
			vertexList.push_back(prevRing[j]); //1
			vertexList.push_back(thisRing[j]); //2

			vertexList.push_back(origin); //3
			vertexList.push_back(prevRing2[j]); //4
			vertexList.push_back(thisRing2[j]); //5

			prevRing[j].position = thisRing[j].position;
			prevRing[j].color = thisRing[j].color;

			prevRing2[j].position = thisRing2[j].position;
			prevRing2[j].color = thisRing2[j].color;

			origin.texture = XMFLOAT2(0.0f, 0.0f);
			prevRing[j].texture = XMFLOAT2(0.5f, 1.0f);
			thisRing[j].texture = XMFLOAT2(1.0f, 0.0f);

			prevRing2[j].texture = XMFLOAT2(0.5f, 1.0f);
			thisRing2[j].texture = XMFLOAT2(1.0f, 0.0f);

			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 1);
			indexList.push_back(segmentsAdded + 2);

			//and the reverse
			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 2);
			indexList.push_back(segmentsAdded + 1);
			segmentsAdded += 3;

			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 1);
			indexList.push_back(segmentsAdded + 2);

			//and the reverse
			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 2);
			indexList.push_back(segmentsAdded + 1);
			segmentsAdded += 3;
		}

		//skip a ring for texturing purposes
		//segmentsAdded += leaves;
	}

	float pitch = transform.rot.x * 0.0174532925f;
	float yaw = transform.rot.y * 0.0174532925f;
	float roll = transform.rot.z * 0.0174532925f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	for (int i = 0; i < indexList.size(); i += 3)
	{
		XMFLOAT3 normal = ComputeNormal(vertexList[indexList[i]].position,
			vertexList[indexList[i + 1]].position, vertexList[indexList[i + 2]].position);

		vertexList[indexList[i]].normal = normal;
		vertexList[indexList[i + 1]].normal = normal;
		vertexList[indexList[i + 2]].normal = normal;
	}

	for (auto it = vertexList.begin(); it != vertexList.end(); it++)
	{
		it->position = MultFloat3(it->position, transform.scale);
		XMVECTOR tempPos = XMLoadFloat3(&it->position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		XMStoreFloat3(&it->position, tempPos);
		it->position = AddFloat3(it->position, transform.pos);
	}

	ModelClass *newModel = new ModelClass();
	newModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), vertexList, indexList, "../Procedural-ATP/texture.tga", rose_tex_param);
	m_objects.push_back(newModel);
}

Transform GraphicsClass::CreateTrunk(Transform transform, Parameters param, int recursion)
{
	std::vector<ModelClass::VertexType> vertexList;
	std::vector<unsigned long> indexList;

	XMFLOAT3 newRot;
	newRot.x = 0.0f;
	newRot.y = 0.0f;
	newRot.z = 0.0f;

	XMFLOAT3 up;
	up.x = 0.0f;
	up.y = param.segHeight;
	up.z = 0.0f;

	XMFLOAT3 tempUp;
	tempUp.x = 0.0f;
	tempUp.y = 0.0f;
	tempUp.z = 0.0f;

	int ring = param.rings;
	int segments = param.segments;

	float localScale = param.startWidth;
	ModelClass::VertexType* thisRing = new ModelClass::VertexType[ring];
	ModelClass::VertexType* prevRing = new ModelClass::VertexType[ring];

	float angle = (360.0f / ring) * 0.0174532925f;

	//initial ring
	for (int i = 0; i < ring; i++)
	{
		prevRing[i].position = XMFLOAT3(param.startWidth, 0.0f, 0.0f);
		XMMATRIX rotMat = XMMatrixRotationY(angle * i);
		XMVECTOR tempPos = XMLoadFloat3(&prevRing[i].position);
		tempPos = XMVector3TransformCoord(tempPos, rotMat);
		XMStoreFloat3(&prevRing[i].position, tempPos);

		//prevRing[i].position = AddFloat3(prevRing[i].position, origin);

		//prevRing[i].position = origin;

		prevRing[i].texture = XMFLOAT2((float)i / ring / 1.0f, 0.0f);

		prevRing[i].color = transform.color;
	}

	int segmentsAdded = 0;

	for (int i = 0; i < segments; i++)
	{
		//random rotation
		newRot.x += RandFloat(param.lowerBound, param.upperBound);
		//newRot.y += ((rand() % 10) - 5) / 3.0f;
		newRot.z += RandFloat(param.lowerBound, param.upperBound);

		float pitch = newRot.x * 0.0174532925f;
		float yaw = newRot.y * 0.0174532925f;
		float roll = newRot.z * 0.0174532925f;

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
		XMVECTOR upVector = XMLoadFloat3(&up);

		//rotate the up vector
		upVector = XMVector3TransformCoord(upVector, rotationMatrix);

		XMStoreFloat3(&up, upVector);

		tempUp = AddFloat3(tempUp, up);

		up.x = 0.0f;
		up.y = param.segHeight;
		up.z = 0.0f;

		//origin = AddFloat3(origin, up);

		localScale = param.endWidth + ((segments - (i + 1)) / (float)segments) * (param.startWidth - param.endWidth);

		//set the new ring up
		for (int j = 0; j < ring; j++)
		{
			ModelClass::VertexType nextPos;

			thisRing[j].position = XMFLOAT3(localScale, 0.0f, 0.0f);
			//thisRing[j].position = MultFloat3(thisRing[j].position, XMFLOAT3(scale, scale, scale));
			XMMATRIX rotMat = XMMatrixRotationY(angle * j);
			XMMATRIX rotMat2 = XMMatrixRotationY(angle);
			XMVECTOR tempPos = XMLoadFloat3(&thisRing[j].position);
			tempPos = XMVector3TransformCoord(tempPos, rotMat);
			//tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
			XMStoreFloat3(&thisRing[j].position, tempPos);

			tempPos = XMVector3TransformCoord(tempPos, rotMat2);
			XMStoreFloat3(&nextPos.position, tempPos);

			thisRing[j].position = AddFloat3(tempUp, thisRing[j].position);
			nextPos.position = AddFloat3(tempUp, nextPos.position);
			//thisRing[j].position.x += ((rand() % 10) - 5) / 20;
			//thisRing[j].position.y += ((rand() % 10) - 5) / 20;
			//thisRing[j].position.z += ((rand() % 10) - 5) / 20;

			//thisRing[j].color = prevRing[j].color;
			thisRing[j].color = transform.color;
			thisRing[j].color.x *= 1 + 0.75f * localScale;
			thisRing[j].color.y *= 1 + 0.75f * localScale;
			thisRing[j].color.z *= 1 + 0.75f * localScale;

			nextPos.color = thisRing[j].color;

			thisRing[j].texture = XMFLOAT2((float)j / ring / 1.0f, (float)(i + 1) / segments);
			nextPos.texture = XMFLOAT2((float)(j + 1) / ring / 1.0f, (float)(i + 1) / segments);

			vertexList.push_back(prevRing[j]);
			vertexList.push_back(thisRing[j]);
			vertexList.push_back(nextPos);

			vertexList.push_back(prevRing[j]);
			vertexList.push_back(nextPos);

			//this bit's necessary for when it does wrap around to 0, since the texture coordinate X will be 0
			int temp = (j + 1) % ring;
			prevRing[temp].texture = XMFLOAT2((float)(j + 1) / ring / 1.0f, (float)i / segments);
			vertexList.push_back(prevRing[temp]);
			prevRing[temp].texture = XMFLOAT2((float)temp / ring / 1.0f, (float)i / segments);

			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 2);
			indexList.push_back(segmentsAdded + 1);

			segmentsAdded += 3;

			indexList.push_back(segmentsAdded);
			indexList.push_back(segmentsAdded + 2);
			indexList.push_back(segmentsAdded + 1);

			segmentsAdded += 3;
		}

		bool toRecurse = false;
		int chance = 50;

		if (recursion > 0 && rand() % 100 < chance && i < (float)segments / 3.0f)
		{
			toRecurse = true;
			recursion--;
		}

		XMFLOAT3 average = zero;

		//update previous ring
		for (int j = 0; j < ring; j++)
		{
			prevRing[j].position = thisRing[j].position;
			prevRing[j].color = transform.color;
			prevRing[j].texture = thisRing[j].texture;

			if (toRecurse)
			{
				average = AddFloat3(average, prevRing[j].position);
			}
		}

		if (toRecurse)
		{
			average = ScalarFloat3(average, 1.0f / (float)ring);

			float pitch = transform.rot.x * 0.0174532925f;
			float yaw = transform.rot.y * 0.0174532925f;
			float roll = transform.rot.z * 0.0174532925f;

			XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

			average = MultFloat3(average, transform.scale);
			XMVECTOR tempPos = XMLoadFloat3(&average);
			tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
			XMStoreFloat3(&average, tempPos);
			average = AddFloat3(average, transform.pos);

			Transform temp = transform;
			temp.scale = MultFloat3(temp.scale, XMFLOAT3(localScale * 0.5, localScale * 0.95, localScale * 0.5));

			temp.pos = average;
			temp.rot = AddFloat3(temp.rot, XMFLOAT3(RandFloat(0.0f, 25.0f), RandFloat(0.0f, 360.0f), RandFloat(0.0f, 20.0f)));

			temp = CreateTrunk(temp, param, recursion);
			//CreateLeaves(temp, leaf_param);
		}
	}

	//add the top
	/*segmentsAdded -= ring;
	for (int i = 0; i < ring; i++)
	{
		vertexList.push_back(thisRing[i]);
		indexList.push_back(segmentsAdded);
		if ((segmentsAdded + 1) % ring == 0)
		{
			indexList.push_back(segmentsAdded + 1 - ring);
		}
		else
		{
			indexList.push_back(segmentsAdded + 1);
		}
		segmentsAdded++;
	}*/

	float pitch = transform.rot.x * 0.0174532925f;
	float yaw = transform.rot.y * 0.0174532925f;
	float roll = transform.rot.z * 0.0174532925f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	for (auto it = vertexList.begin(); it != vertexList.end(); it++)
	{
		it->position = MultFloat3(it->position, transform.scale);
		XMVECTOR tempPos = XMLoadFloat3(&it->position);
		tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
		XMStoreFloat3(&it->position, tempPos);
		it->position = AddFloat3(it->position, transform.pos);
	}

	for (int i = 0; i < indexList.size(); i += 3)
	{
		XMFLOAT3 normal = ComputeNormal(vertexList[indexList[i]].position,
			vertexList[indexList[i + 1]].position, vertexList[indexList[i + 2]].position);

		vertexList[indexList[i]].normal = normal;
		vertexList[indexList[i + 1]].normal = normal;
		vertexList[indexList[i + 2]].normal = normal;
	}

	XMVECTOR tempPos = XMLoadFloat3(&tempUp);
	tempPos = XMVector3TransformCoord(tempPos, rotationMatrix);
	XMStoreFloat3(&tempUp, tempPos);

	ModelClass *newModel = new ModelClass();
	newModel->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), vertexList, indexList, "../Procedural-ATP/bark.tga", trunk_tex_param);
	m_objects.push_back(newModel);

	tempUp = MultFloat3(tempUp, transform.scale);

	return Transform(AddFloat3(tempUp, transform.pos), transform.scale, AddFloat3(transform.rot, newRot), transform.color);
}

void GraphicsClass::CreateTree(Transform transform, Parameters param)
{
	Transform result = CreateTrunk(transform, param, 3);
	CreateLeaves(result, param);
}

void GraphicsClass::CreateBush(Transform transform, Parameters param)
{
	int numStems = 50;
	for (int i = 0; i < numStems; i++)
	{
		Transform temp = transform;
		temp.pos.x += RandFloat(param.lowerBound, param.upperBound);
		temp.pos.z += RandFloat(param.lowerBound, param.upperBound);
		temp = CreateTrunk(temp, trunk_param, 3);
		CreateLeaves(temp, leaf_param);
	}
}

void GraphicsClass::PopModel(bool clearAll)
{
	m_objects.back()->Shutdown();
	m_objects.pop_back();

	if (clearAll)
	{
		while (m_objects.size() > 1)
		{
			m_objects.back()->Shutdown();
			m_objects.pop_back();
		}
	}
}

void GraphicsClass::QueueRemake()
{
	remaking = true;
}

void GraphicsClass::RemakeSystem()
{
	m_hiresMap->Shutdown();

	for each(Planet* planet in m_planets)
	{
		if (!planet->Built())
		{
			remaking = true;
			return;
		}
	}

	remaking = false;

	for(int i = 0; i < m_planets.size(); i++)
	{
		if (!m_planets[i]->Shutdown())
		{
			remaking = true;
			return;
		}
		delete m_planets[i];
	}
	m_planets.clear();

	StarParam new_star;

	if (!randomStar)
	{
		new_star = starParam;
	}

	if (m_star)
	{
		m_star->Shutdown();
		delete m_star;
	}

	//delete m_hiresMap;

	//m_hiresMap = new Mapping();

	m_star = new Star();
	if (randomStar)
	{
		new_star.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		new_star.temperature = Maths::RandFloat(2000.0f, 10000.0f); //6000.0f;
		new_star.radius = Maths::RandFloat(700000.0f, 1000000.0f); //700000.0f;
	}
	m_star->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), new_star);

	starParam = new_star;

	m_light->SetLightDiffuse(m_star->GetColor());

	float angle = 0.0f;
	float z = 0.0f;

	for (int i = 0; i < numPlanets; i++)
	{
		Planet* new_planet = new Planet();

		z += RandFloatSeeded(100.0f, 130.0f, rand());

		XMFLOAT3 newPos = XMFLOAT3(0, 0, z);

		angle += RandFloatSeeded(45.0f, 95.0f, rand());

		XMMATRIX rotMat = XMMatrixRotationY(angle * 0.0174532925f);
		XMVECTOR tempPos = XMLoadFloat3(&newPos);
		tempPos = XMVector3TransformCoord(tempPos, rotMat);
		XMStoreFloat3(&newPos, tempPos);

		float pX = RandFloatSeeded(-500.0f, 500.0f, rand());
		float pY = RandFloatSeeded(-500.0f, 500.0f, rand());
		float pZ = RandFloatSeeded(-500.0f, 500.0f, rand());

		float sX = RandFloatSeeded(0.0f, 1.0f, rand());
		float sY = RandFloatSeeded(0.0f, 1.0f, rand());
		float sZ = RandFloatSeeded(0.0f, 1.0f, rand());

		float waterHeight = Maths::RandIntSeeded(0, 100, (int)(pX + pY + pZ));
		waterHeight /= 100.0f;

		float flatten = Maths::RandFloatSeeded(1.5f, 4.5f, pX + pY + pZ);

		new_planet->Initialize(Maths::AddFloat3(m_star->GetParam().pos, newPos), RandFloatSeeded(20.0f, 50.0f, rand()), XMFLOAT3(pX, pY, pZ), XMFLOAT3(pX, pY, pZ),
			XMFLOAT4(sX, sY, sZ, 1.0f), m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), m_star->GetParam(), m_hiresMap, waterHeight, flatten, -1);
		m_planets.push_back(new_planet);
	}
}

void GraphicsClass::RemakeStar()
{
	m_hiresMap->Shutdown();

	for each(Planet* planet in m_planets)
	{
		if (!planet->Built())
		{
			remakingStar = true;
			return;
		}
	}

	remakingStar = false;

	for (int i = 0; i < m_planets.size(); i++)
	{
		XMFLOAT3 newPos = m_planets[i]->GetPosition();
		float newSize = m_planets[i]->GetSize();
		XMFLOAT3 newPerlin = m_planets[i]->GetPerlin();
		XMFLOAT3 newMapPerlin = m_planets[i]->GetMapPerlin();
		XMFLOAT4 newSky = m_planets[i]->GetUnscaledSky();
		float newWaterHeight = m_planets[i]->GetWaterHeight();
		float newFlatten = m_planets[i]->GetFlat();
		float newTemperature = m_planets[i]->GetTemperature();
		if (!m_planets[i]->Shutdown())
		{
			remakingStar = true;
			return;
		}
		delete m_planets[i];

		//remake here
		m_planets[i] = new Planet();
		m_planets[i]->Initialize(newPos, newSize, newPerlin, newMapPerlin, newSky, m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), starParam,
			m_hiresMap, newWaterHeight, newFlatten, -1);
	}
}

void GraphicsClass::QueueRemakeStar()
{
	remakingStar = true;

	StarParam new_star;

	if (!randomStar)
	{
		new_star = starParam;
	}

	if (m_star)
	{
		m_star->Shutdown();
		delete m_star;
	}

	m_star = new Star();
	if (randomStar)
	{
		new_star.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		new_star.temperature = Maths::RandFloat(2000.0f, 10000.0f); //6000.0f;
		new_star.radius = Maths::RandFloat(700000.0f, 1000000.0f); //700000.0f;
	}
	m_star->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), new_star);

	starParam = new_star;

	m_light->SetLightDiffuse(m_star->GetColor());
}

void GraphicsClass::RemakePlanet()
{
	if (!m_remakingPlanet->Built()) return;

	remakingPlanet = false;

	if (!m_remakingPlanet->Shutdown())
	{
		remakingPlanet = true;
		return;
	}

	//delete m_remakingPlanet;

	//Planet* newPlanet = new Planet();

	m_remakingPlanet->Initialize(planetParam.p_position, planetParam.p_size, planetParam.p_seed, planetParam.m_seed, planetParam.p_sky, m_Direct3D->GetDevice(),
		m_Direct3D->GetDeviceContext(), m_star->GetParam(), m_hiresMap, planetParam.m_waterHeight, planetParam.p_flat, planetParam.p_temperature);

	//m_remakingPlanet = newPlanet;

	builtPlanet = NULL;
}

void GraphicsClass::QueueRemakePlanet()
{
	if (builtPlanet == NULL) return;
	m_remakingPlanet = builtPlanet;

	remakingPlanet = true;
}

void TW_CALL Trunk(void *p)
{
	GraphicsClass* parent = static_cast<GraphicsClass*>(p);
	parent->PopModel(true);
	parent->CreateTrunk(parent->new_transform, parent->trunk_param, parent->trunk_param.recursion);
}

void TW_CALL MakeSystem(void *p)
{
	GraphicsClass* parent = static_cast<GraphicsClass*>(p);
	parent->RemakeSystem();
}

void TW_CALL MakePlanet(void *p)
{
	GraphicsClass* parent = static_cast<GraphicsClass*>(p);
	parent->QueueRemakePlanet();
}

void TW_CALL MakeStar(void *p)
{
	GraphicsClass* parent = static_cast<GraphicsClass*>(p);
	parent->QueueRemakeStar();
}

void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
	// Copy the content of souceString handled by the AntTweakBar library to destinationClientString handled by your application
	destinationClientString = sourceLibraryString;
}

void GraphicsClass::SetTweakBar(TwBar* bar)
{
	TwCopyStdStringToClientFunc(CopyStdStringToClient);

	TwAddVarRO(bar, "Triangle count", TW_TYPE_INT32, &triCount, " group='Debug info' ");

	TwAddSeparator(bar, "", NULL);

	//star settings
	TwAddButton(bar, "Remake star", MakeStar, this, " group='Star settings'");
	TwAddVarRW(bar, "Star temp", TW_TYPE_FLOAT, &starParam.temperature, " min=1000 max=10000 group='Star settings' ");
	TwAddVarRW(bar, "Star radius", TW_TYPE_FLOAT, &starParam.radius, " min=500000 max=1000000 group='Star settings' ");
	TwAddVarRW(bar, "Random star", TW_TYPE_BOOLCPP, &randomStar, " group='Star settings' ");

	//planet settings
	TwAddButton(bar, "Remake planet", MakePlanet, this, " group='Planet settings'");
	TwAddVarRW(bar, "Temperature", TW_TYPE_FLOAT, &planetParam.p_temperature, " min=0 max=3000 group='Planet settings' ");
	TwAddVarRW(bar, "Radius", TW_TYPE_FLOAT, &planetParam.p_size, " min=10 max=60 group='Planet settings' ");
	TwAddVarRW(bar, "Sky", TW_TYPE_COLOR3F, &planetParam.p_sky, " group='Planet settings' ");
	TwAddVarRW(bar, "Terrain height", TW_TYPE_FLOAT, &planetParam.p_flat, " min=0.5 max=9.5 step=0.1 group='Planet settings' ");
	TwAddVarRW(bar, "Seed X", TW_TYPE_FLOAT, &planetParam.p_seed.x, " group='Seed' ");
	TwAddVarRW(bar, "Seed Y", TW_TYPE_FLOAT, &planetParam.p_seed.y, " group='Seed' ");
	TwAddVarRW(bar, "Seed Z", TW_TYPE_FLOAT, &planetParam.p_seed.z, " group='Seed' ");
	TwAddVarRW(bar, "Map seed X", TW_TYPE_FLOAT, &planetParam.m_seed.x, " group='Seed' ");
	TwAddVarRW(bar, "Map seed Y", TW_TYPE_FLOAT, &planetParam.m_seed.y, " group='Seed' ");
	TwAddVarRW(bar, "Map seed Z", TW_TYPE_FLOAT, &planetParam.m_seed.z, " group='Seed' ");
	//TwAddVarRW(bar, "Pos X", TW_TYPE_FLOAT, &planetParam.p_position.x, " group='Position' ");
	//TwAddVarRW(bar, "Pos Y", TW_TYPE_FLOAT, &planetParam.p_position.y, " group='Position' ");
	//TwAddVarRW(bar, "Pos Z", TW_TYPE_FLOAT, &planetParam.p_position.z, " group='Position' ");
	TwAddVarRW(bar, "Water height", TW_TYPE_FLOAT, &planetParam.m_waterHeight, " min=0.0 max=1.0 step=0.01 group='Planet settings' ");

	TwDefine(" Options/'Position'   group='Planet settings' ");
	TwDefine(" Options/'Seed'   group='Planet settings' ");
	TwDefine(" Options/'Random'   group='Planet settings' ");

	TwAddSeparator(bar, "NewSep", NULL);

	TwAddVarRW(bar, "Planet count", TW_TYPE_INT32, &numPlanets, " min=1 max=20 ");
	TwAddButton(bar, "Remake system", MakeSystem, this, " ");

	TwAddButton(bar, "Click and hold to pan view", NULL, NULL, NULL);
	TwAddButton(bar, "WASD to move, SHIFT for speed", NULL, NULL, NULL);
	TwAddButton(bar, "SPACE/CTRL for height", NULL, NULL, NULL);

	TwDefine(" Options/'Star settings' opened=false ");
	TwDefine(" Options/'Planet settings' opened=false ");
	TwDefine(" Options/'Seed' opened=false ");
	TwDefine(" Options/'Position' opened=false ");
	TwDefine(" Options/'Random' opened=false ");
}