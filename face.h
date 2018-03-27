#pragma once

#include "modelclass.h"
#include <list>
#include <thread>
#include <noise/noise.h>
#include "noiseutils.h"
#include "maths.h"
#include "mapping.h"

using namespace noise;
using namespace Maths;

class Face
{
public:
	struct Transform
	{
		Transform() {};
		Transform(XMFLOAT3 _pos, XMFLOAT3 _scale, XMFLOAT3 _rot, XMFLOAT3 _perlin) :Transform()
		{
			pos = _pos;
			scale = _scale;
			rot = _rot;
			perlin = _perlin;
		}
		XMFLOAT3 pos;
		XMFLOAT3 scale;
		XMFLOAT3 rot;
		XMFLOAT3 perlin;
	};

	struct Triangle
	{
		Triangle() {};
		Triangle(int _v1, int _v2, int _v3) :Triangle()
		{
			v1 = _v1;
			v2 = _v2;
			v3 = _v3;
		}
		int v1, v2, v3;
	};

public:
	Face();
	~Face();

	bool Initialize(Transform transform, int recursion, int faceDir, float distance,
		ID3D11Device* device, ID3D11DeviceContext* context, XMFLOAT3 offset, float size,
		int divide, Mapping* map, Mapping* hires);
	void MakeFace(ID3D11Device* device, ID3D11DeviceContext* context);
	int GetIndexCount(float distance);
	void Shutdown();

	std::list<ModelClass*> GetModels(XMFLOAT3 camPos, ID3D11Device* device, ID3D11DeviceContext* context);

	XMFLOAT3 GetPos();

	void Rebuild(ID3D11Device* device, ID3D11DeviceContext* context);
	bool IsHires();

private:

	void LoadModel();
	void UnloadModel();
	bool Created();
	bool Creating();
	void CreateChildren(ID3D11Device* device, ID3D11DeviceContext* context);
	void SetParent(Face* set);

	//recursion for LoD
	Face* m_children[4];
	Face* m_parent;
	ModelClass* m_model;
	float viewDist;
	XMFLOAT3 m_pos;
	bool created = false;
	bool creating = false;

	//data to recreate models
	Transform m_transform;
	int m_recursion;
	int m_faceDir;
	float m_distance;
	XMFLOAT3 m_offset;
	float m_size;
	int m_divide;
	//float m_temperature;
	//float* m_map;
	//int m_res;
	bool hasHires;

	Mapping* lowres_map;
	Mapping* hires_map;
	Mapping* m_map;
};