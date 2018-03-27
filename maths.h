#pragma once

#include <DirectXMath.h>
#include <chrono>
using namespace DirectX;

namespace Maths
{
	// ------------------
	// Maths functions
	// ------------------

	//float shorthands
	const XMFLOAT3 one = XMFLOAT3(1.0f, 1.0f, 1.0f);
	const XMFLOAT3 half = XMFLOAT3(0.5f, 0.5f, 0.5f);
	const XMFLOAT3 quarter = XMFLOAT3(0.25f, 0.25f, 0.25f);
	const XMFLOAT3 zero = XMFLOAT3(0.0f, 0.0f, 0.0f);

	XMFLOAT3 AddFloat3(XMFLOAT3 a, XMFLOAT3 b);
	XMFLOAT3 TakeFloat3(XMFLOAT3 a, XMFLOAT3 b);
	XMFLOAT3 MultFloat3(XMFLOAT3 a, XMFLOAT3 b);
	XMFLOAT3 ScalarFloat3(XMFLOAT3 a, float b);
	XMFLOAT4 ScalarFloat4(XMFLOAT4 a, float b);

	//https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.geometric.xmvector3normalize(v=vs.85).aspx
	XMVECTOR XMVector3NormalizeRobust(FXMVECTOR V);
	XMFLOAT3 ComputeNormal(XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c);

	float RandFloat(float min, float max);
	int RandInt(int min, int max);
}