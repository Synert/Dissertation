#include "maths.h"

namespace Maths
{
	XMFLOAT3 AddFloat3(XMFLOAT3 a, XMFLOAT3 b)
	{
		XMFLOAT3 result;
		result.x = a.x + b.x;
		result.y = a.y + b.y;
		result.z = a.z + b.z;

		return result;
	}

	XMFLOAT3 TakeFloat3(XMFLOAT3 a, XMFLOAT3 b)
	{
		XMFLOAT3 result;
		result.x = a.x - b.x;
		result.y = a.y - b.y;
		result.z = a.z - b.z;

		return result;
	}

	XMFLOAT3 MultFloat3(XMFLOAT3 a, XMFLOAT3 b)
	{
		XMFLOAT3 result;
		result.x = a.x * b.x;
		result.y = a.y * b.y;
		result.z = a.z * b.z;

		return result;
	}

	XMFLOAT3 ScalarFloat3(XMFLOAT3 a, float b)
	{
		XMFLOAT3 result = a;
		result.x *= b;
		result.y *= b;
		result.z *= b;

		return result;
	}

	XMFLOAT4 ScalarFloat4(XMFLOAT4 a, float b)
	{
		XMFLOAT4 result = a;
		result.x *= b;
		result.y *= b;
		result.z *= b;

		return result;
	}

	//https://msdn.microsoft.com/en-us/library/windows/desktop/microsoft.directx_sdk.geometric.xmvector3normalize(v=vs.85).aspx
	XMVECTOR XMVector3NormalizeRobust(FXMVECTOR V)
	{
		// Compute the maximum absolute value component.
		XMVECTOR vAbs = XMVectorAbs(V);
		XMVECTOR max0 = XMVectorSplatX(vAbs);
		XMVECTOR max1 = XMVectorSplatY(vAbs);
		XMVECTOR max2 = XMVectorSplatZ(vAbs);
		max0 = XMVectorMax(max0, max1);
		max0 = XMVectorMax(max0, max2);

		// Divide by the maximum absolute component.
		XMVECTOR normalized = XMVectorDivide(V, max0);

		// Set to zero when the original length is zero.
		XMVECTOR mask = XMVectorNotEqual(g_XMZero, max0);
		normalized = XMVectorAndInt(normalized, mask);

		XMVECTOR t0 = XMVector3LengthSq(normalized);
		XMVECTOR length = XMVectorSqrt(t0);

		// Divide by the length to normalize.
		normalized = XMVectorDivide(normalized, length);

		// Set to zero when the original length is zero or infinity.  In the
		// latter case, this is considered to be an unexpected condition.
		normalized = XMVectorAndInt(normalized, mask);
		return normalized;
	}

	XMFLOAT3 ComputeNormal(XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c)
	{
		//first, convert all of these to XMVECTORs

		XMVECTOR aVec = XMLoadFloat3(&a);
		XMVECTOR bVec = XMLoadFloat3(&b);
		XMVECTOR cVec = XMLoadFloat3(&c);

		XMVECTOR resultVec = XMVector3NormalizeRobust(XMVector3Cross(bVec - aVec, cVec - aVec));

		XMFLOAT3 result;

		XMStoreFloat3(&result, resultVec);

		return result;
	}

	float RandFloat(float min, float max)
	{
		srand(time(NULL));

		float result = min;
		float dif = max - min;

		float mult = (float)(rand() % 100) / 100.0f;

		result += dif * mult;

		return result;
	}

	int RandInt(int min, int max)
	{
		srand(time(NULL));

		int result = min;
		int dif = max - min;

		result += (rand() % dif);

		return result;
	}

	float RandFloatSeeded(float min, float max, int seed)
	{
		srand(seed);

		float result = min;
		float dif = max - min;

		float mult = (float)(rand() % 10000) / 10000.0f;

		result += dif * mult;

		return result;
	}

	int RandIntSeeded(int min, int max, int seed)
	{
		srand(seed);

		int result = min;
		int dif = max - min;

		result += (rand() % dif);

		return result;
	}

	XMFLOAT3 Float4To3(XMFLOAT4 in)
	{
		XMFLOAT3 result;
		result.x = in.x;
		result.y = in.y;
		result.z = in.z;

		return result;
	}

	XMFLOAT4 Float3To4(XMFLOAT3 in)
	{
		XMFLOAT4 result;
		result.x = in.x;
		result.y = in.y;
		result.z = in.z;

		return result;
	}
}