#pragma once

#include "maths.h"
#include "modelclass.h"
#include <list>

struct StarParam
{
	StarParam() {};
	StarParam(XMFLOAT3 _pos, float _radius, float _temperature)
	{
		pos = _pos;
		radius = _radius;
		temperature = _temperature;
	}

	XMFLOAT3 pos;
	float radius;
	float temperature;
};

class Star
{
public:
	Star();
	~Star();

	void Initialize(ID3D11Device* device, ID3D11DeviceContext* context, StarParam param);
	ModelClass* GetModel();
	void Shutdown();
	StarParam GetParam();
	XMFLOAT4 GetColor();

	//for the GUI to access
	StarParam m_param;

private:
	ModelClass* m_model;
	XMFLOAT4 m_color;
};