#pragma once

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "colorshaderclass.h"
#include "colors.h"
#include "textureshaderclass.h"
#include "light.h"

#include <chrono>
#include <fstream>
#include <list>
#include <vector>
#include <AntTweakBar.h>
#include <thread>

#include <noise/noise.h>
#include "noiseutils.h"

#include "face.h"
#include "planet.h"
#include "maths.h"
#include "mapping.h"
#include "Star.h"

using namespace noise;

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = false; //evil
const float SCREEN_DEPTH = 20000.0f;
const float SCREEN_NEAR = 0.1f;

struct Transform
{
	Transform() {};
	Transform(XMFLOAT3 _pos, XMFLOAT3 _scale, XMFLOAT3 _rot, XMFLOAT4 _color):Transform()
	{
		pos = _pos;
		scale = _scale;
		rot = _rot;
		color = _color;
	}
	XMFLOAT3 pos;
	XMFLOAT3 scale;
	XMFLOAT3 rot;
	XMFLOAT4 color;
};

struct Parameters
{
	float upperBound;
	float lowerBound;
	int segments;
	int rings;
	float startWidth;
	float endWidth;
	float segHeight;

	float upperBoundX;
	float lowerBoundX;

	float upperBoundY;
	float lowerBoundY;

	float upperBoundZ;
	float lowerBoundZ;

	int recursion;
};

struct PlanetParam
{
	float p_temperature;
	XMFLOAT4 p_sky;
	XMFLOAT3 p_seed;
	XMFLOAT3 p_position;
	float p_size;
	float p_flat;

	float m_waterHeight;
	XMFLOAT3 m_seed;
};

class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND, CameraClass*);
	void Shutdown();
	bool Frame(TwBar* bar);
	void SetTweakBar(TwBar* bar);

	//generation functions
	//this is left over from a previous assignment
	void CreateCube(Transform transform);
	void CreateSphere(Transform transform, int divide);
	void CreateTriangle(Transform transform);
	void CreateLeaves(Transform transform, Parameters param);
	void CreateMoreLeaves(Transform transform, Parameters param);
	void CreateRose(Transform transform, Parameters param);
	Transform CreateTrunk(Transform transform, Parameters param, int recursion);
	void CreateBush(Transform transform, Parameters param);
	void CreateTree(Transform transform, Parameters param);

	void QueueRemake();
	void RemakeSystem();
	void QueueRemakePlanet();

	void RemakePlanet();

	void PopModel(bool clearAll);
	void ExportModel();

	float GetSpeed(XMFLOAT3 position);

	void RenderModel(ModelClass* model);

	Transform new_transform;

	Parameters trunk_param;
	Parameters leaf_param;
	Parameters rose_param;

	TexParam tex_param;
	TexParam leaf_tex_param;
	TexParam trunk_tex_param;
	TexParam rose_tex_param;

	string export_path;

	StarParam starParam;
	bool randomStar = true;
	bool randomPlanets = true;
	int numPlanets = 10;
	PlanetParam planetParam;

	bool remakingPlanet = false;

private:
	bool Render(TwBar* bar);

private:
	D3DClass* m_Direct3D;
	ModelClass* m_Model;
	std::vector<ModelClass*> m_objects;
	std::vector<Planet*> m_planets;
	Star* m_star;
	ColorShaderClass* m_ColorShader;
	TextureShaderClass* m_TextureShader;
	CameraClass* m_Camera;
	Light* m_light;
	int triCount;
	bool remaking = false;

	Planet* builtPlanet;
	Planet* m_remakingPlanet;
	Mapping* m_hiresMap;
};