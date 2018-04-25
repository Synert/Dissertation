#include "mapping.h"

Mapping::Mapping()
{
	h_built = false;
	h_building = false;
}

void Mapping::Setup(int _hres, float m_temp, XMFLOAT3 perlin, XMFLOAT3 mapPerlin, float waterHeight, float flatten)
{
	if (h_building) return;
	if (h_built)
	{
		Shutdown();
	}
	h_building = true;
	cancel = false;

	h_map = NULL;
	c_map = NULL;

	std::thread new_thread(&Mapping::CreateMaps, this, _hres, m_temp, perlin, mapPerlin, waterHeight, flatten);
	new_thread.detach();
	//new_thread.join();
}

void Mapping::CreateMaps(int _hres, float m_temp, XMFLOAT3 perlin, XMFLOAT3 mapPerlin, float waterHeight, float flatten)
{
	h_res = _hres;
	c_map = new XMFLOAT3[6 * h_res * h_res];
	h_map = new float[6 * h_res * h_res];
	CreateHeightMap(m_temp, perlin, mapPerlin, waterHeight, flatten);
}

void Mapping::CreateHeightMap(float m_temp, XMFLOAT3 perlin, XMFLOAT3 mapPerlin, float waterHeight, float flatten)
{
	h_finished = 0;
	std::thread new_thread1(&Mapping::HeightmapThread, this, 0, m_temp, perlin, mapPerlin, waterHeight, flatten);
	std::thread new_thread2(&Mapping::HeightmapThread, this, 1, m_temp, perlin, mapPerlin, waterHeight, flatten);
	std::thread new_thread3(&Mapping::HeightmapThread, this, 2, m_temp, perlin, mapPerlin, waterHeight, flatten);
	std::thread new_thread4(&Mapping::HeightmapThread, this, 3, m_temp, perlin, mapPerlin, waterHeight, flatten);
	std::thread new_thread5(&Mapping::HeightmapThread, this, 4, m_temp, perlin, mapPerlin, waterHeight, flatten);
	std::thread new_thread6(&Mapping::HeightmapThread, this, 5, m_temp, perlin, mapPerlin, waterHeight, flatten);
	new_thread1.join();
	new_thread2.join();
	new_thread3.join();
	new_thread4.join();
	new_thread5.join();
	new_thread6.join();

	h_built = true;
	cancel = false;
}

void Mapping::HeightmapThread(int z, float m_temp, XMFLOAT3 perlin, XMFLOAT3 mapPerlin, float waterHeight, float flatten)
{
	module::Perlin perlinModule, waterModule, terrainPicker;
	module::RidgedMulti altModule;
	module::Billow billowModule;
	module::Blend myModule;
	terrainPicker.SetFrequency(0.5f);
	terrainPicker.SetPersistence(0.25f);
	altModule.SetOctaveCount(7);
	billowModule.SetOctaveCount(8);
	perlinModule.SetOctaveCount(10);
	waterModule.SetOctaveCount(5);
	myModule.SetSourceModule(0, perlinModule);
	myModule.SetSourceModule(1, altModule);
	myModule.SetSourceModule(2, billowModule);
	myModule.SetControlModule(terrainPicker);

	XMFLOAT3 baseRock = XMFLOAT3(Maths::RandInt(20, 85), Maths::RandInt(10, 40), Maths::RandInt(10, 55));
	XMFLOAT3 baseGrass = XMFLOAT3(Maths::RandInt(0, 35), Maths::RandInt(30, 80), Maths::RandInt(0, 50));

	baseRock = Maths::ScalarFloat3(baseRock, 0.01f);
	baseGrass = Maths::ScalarFloat3(baseGrass, 0.01f);

	float pScale = 0.75f;
	XMFLOAT3 perlinScale = XMFLOAT3(pScale, pScale, pScale);

	m_waterHeight = waterHeight;
	//float waterHeight = 0.27f;

	//experimental texture loading
	//https://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file

	FILE* f = fopen("biomes.bmp", "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header

											   // extract image height and width from header
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];

	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);

	for (int i = 0; i < size; i += 3)
	{
		unsigned char tmp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = tmp;
	}

	if (m_temp > 373.2f && m_temp < 1000.0f)
	{
		waterHeight -= (m_temp - 373.2f) / 100.0f;
		if (waterHeight < 0.0f) waterHeight = 0.0f;
	}
	if (m_temp >= 1000.0f)
	{
		waterHeight = 0.3f + (m_temp - 1000.0f) / 1000.0f;
		if (waterHeight > 1.0f) waterHeight = 1.0f;
	}

	for (int x = 0; x < h_res; x++)
	{
		for (int y = 0; y < h_res; y++)
		{
			XMFLOAT3 coord, mapCoord;

			int tempZ = -1.0f;
			float tempX = (float)x / (float)(h_res - 1);
			tempX *= 2;
			tempX--;

			float tempY = (float)y / (float)(h_res - 1);
			tempY *= 2;
			tempY--;

			coord.x = tempX;
			coord.y = tempY;
			coord.z = tempZ;

			XMMATRIX cubeRot = XMMatrixRotationY(((90.0f * (float)z)) * 0.0174532925f);
			if (z > 3)
			{
				cubeRot = XMMatrixRotationX((90.0f + (180.0f * (float)(z - 4))) * 0.0174532925f);
			}

			XMVECTOR tempPos = XMLoadFloat3(&coord);
			tempPos = XMVector3TransformCoord(tempPos, cubeRot);
			XMStoreFloat3(&coord, tempPos);

			mapCoord = coord;

			//get the coordinates for the perlin noise
			coord = Maths::AddFloat3(coord, perlin);
			mapCoord = Maths::AddFloat3(mapCoord, mapPerlin);

			//coord = TakeFloat3(coord, origin);

			coord = Maths::MultFloat3(coord, perlinScale);
			mapCoord = Maths::MultFloat3(mapCoord, perlinScale);

			//get a value between 1 and 0
			double value = myModule.GetValue(coord.x, coord.y, coord.z);
			value += 2.0f;
			value /= 3.0f;

			double waterValue = waterModule.GetValue(mapCoord.x * 3.0f, mapCoord.y * 3.0f, mapCoord.z * 3.0f);
			waterValue += 2.0f;
			waterValue /= 3.0f;

			waterValue *= (waterHeight * 0.5f);

			bool desert = false;
			if (!(m_temp < 373.2f && m_temp > 273.2f))
			{
				//waterValue = 0.0f;
				//desert = true;
			}
			if (m_temp > 373.2f)
			{
				waterValue -= (m_temp - 373.2f) / 2700.0f;
				if (waterValue < 0.0f) waterValue = 0.0f;
			}

			//float value = tempX;

			float tempValue = 1.0f - value;
			tempValue += (m_temp - 273.2f) / 2700.0f;

			if (m_temp <= 273.2f)
			{
				tempValue -= (273.2f - m_temp) / 400.0f;
			}

			//tempValue += waterHeight * 2;
			//tempValue /= (1.0f + waterHeight * 2);
			tempValue *= 1.0f + waterHeight * 2.0f;

			//choose the biome
			Biome m_biome = BIOME_NONE;
			if (tempValue < 0.2f && !desert)
			{
				if (waterValue < 0.3f)
				{
					m_biome = BIOME_TUNDRA;
				}
				else m_biome = BIOME_SNOW;
			}
			else if (tempValue <= 0.65f)
			{
				if (waterValue < 0.2f)
				{
					m_biome = BIOME_DESERT_COLD;
				}
				else
				{
					if (value < 0.5f)
					{
						m_biome = BIOME_FOREST_BOREAL;
					}
					else
					{
						if (waterValue < 0.3f)
						{
							m_biome = BIOME_WOODLAND;
						}
						else if (waterValue < 0.6f)
						{
							m_biome = BIOME_FOREST_TEMPERATE_SEASONAL;
						}
						else
						{
							m_biome = BIOME_FOREST_TEMPERATE_RAIN;
						}
					}
				}
			}
			else
			{
				if (waterValue < 0.3f)
				{
					m_biome = BIOME_DESERT_HOT;
				}
				else if (waterValue < 0.6f)
				{
					m_biome = BIOME_FOREST_TROPICAL_SEASONAL;
				}
				else
				{
					m_biome = BIOME_FOREST_TROPICAL_RAIN;
				}
			}

			XMFLOAT3 finalCol = XMFLOAT3(0.0f, 0.0f, 0.0f);

			//m_biome = BIOME_NONE;

			//temporary setup for biome colours
			//to-do: put and/or procgen textures for each, place objects
			switch ((int)m_biome)
			{
			case BIOME_DESERT_COLD:
				finalCol = XMFLOAT3(0.4f, 0.35f, 0.15f);
				break;
			case BIOME_DESERT_HOT:
				finalCol = XMFLOAT3(0.45f, 0.25f, 0.1f);
				break;
			case BIOME_FOREST_BOREAL:
				finalCol = XMFLOAT3(0.45f, 0.75f, 0.25f);
				break;
			case BIOME_FOREST_TEMPERATE_RAIN:
				finalCol = XMFLOAT3(0.05f, 0.65f, 0.35f);
				break;
			case BIOME_FOREST_TEMPERATE_SEASONAL:
				finalCol = XMFLOAT3(0.05f, 0.45f, 0.25f);
				break;
			case BIOME_FOREST_TROPICAL_RAIN:
				finalCol = XMFLOAT3(0.05f, 0.4f, 0.1f);
				break;
			case BIOME_FOREST_TROPICAL_SEASONAL:
				finalCol = XMFLOAT3(0.05f, 0.65f, 0.15f);
				break;
			case BIOME_SNOW:
				//do thing
				finalCol = XMFLOAT3(1.0f, 1.0f, 1.0f);
				break;
			case BIOME_TUNDRA:
				finalCol = XMFLOAT3(0.55f, 0.50f, 0.4f);
				break;
			case BIOME_WOODLAND:
				finalCol = XMFLOAT3(0.45f, 0.65f, 0.35f);
				break;
			}

			finalCol = Maths::ScalarFloat3(finalCol, (value + 2.0f) / 2.0f);

			float cappedValue = tempValue;
			if (cappedValue < 0.0f) cappedValue = 0.0f;
			if (cappedValue > 1.0f) cappedValue = 1.0f;

			float cappedWater = waterValue;
			if (cappedWater < 0.0f) cappedWater = 0.0f;
			if (cappedWater > 1.0f) cappedWater = 1.0f;

			//cappedValue = 1.0f - cappedValue;
			//cappedWater = 1.0f - cappedWater;

			int newTempY = (int)(cappedValue * 100.0f);
			int newTempX = (int)(cappedWater * 100.0f);

			if (newTempX > 99) newTempX = 99;
			if (newTempX < 0) newTempX = 0;

			if (newTempY > 99) newTempY = 99;
			if (newTempY < 0) newTempY = 0;

			finalCol.x = data[3 * (newTempX * width + newTempY)] / 255.0f;
			finalCol.y = data[3 * (newTempX * width + newTempY) + 1] / 255.0f;
			finalCol.z = data[3 * (newTempX * width + newTempY) + 2] / 255.0f;

			float waterColor = 0.0f;
			bool water = false;
			bool lava = false;
			bool ice = false;

			if (m_temp >= 1000.0f && value < waterHeight)
			{
				lava = true;
				waterColor = abs(value - waterHeight);
				waterColor /= waterHeight;
				waterColor = waterHeight - waterColor;
				waterColor += 0.5f;
				waterColor *= 1.0f;
				//newValue *= 0.95f;
			}
			else if (m_temp < 273.2f && value < waterHeight)
			{
				ice = true;
				waterColor = abs(value - waterHeight);
				waterColor /= waterHeight;
				waterColor = waterHeight - waterColor;
				waterColor += 0.5f;
				waterColor *= 1.5f;
			}
			else if (value < waterHeight)
			{
				waterColor = 1.0f - (abs(value - waterHeight) * 2.0f) / waterHeight;
				waterColor /= 2.0f;
				//waterColor /= waterHeight;
				//waterColor = waterHeight - waterColor;
				//waterColor *= 0.5f;
				//newValue = waterHeight;
				//value = waterHeight;
				water = true;
			}

			if (value < waterHeight && (water || ice || lava))
			{
				value = waterHeight;
			}

			//now move that between 1 and 0.15
			float newValue = (float)value;

			float minSize = flatten;

			newValue *= minSize;
			newValue += (10.0f - minSize);
			newValue /= 30.0f;

			float tempScale = 1.0f - (value / 2.5f);
			float newTemp = m_temp * tempScale;

			if (newTemp < 0) newTemp = 0;
			if (newTemp > 3000.0f) newTemp = 3000.0f;

			//XMFLOAT3 col = XMFLOAT3((3000.0f - newTemp) / 3000.0f, newTemp / 3000.0f, 0.0f);
			//col = XMFLOAT3((0.8f - value * 0.8f), (value * 0.7f), 0.5f);
			XMFLOAT3 col = finalCol;

			//base color
			XMFLOAT3 waterCol = XMFLOAT3(0.0f, 0.2f, 0.8f);
			
			//blend with sky- need access to planet first! pass as parameter?
			//waterCol = Blend(waterCol, sky)

			//col = XMFLOAT4(1.0f - value, 0, value, 1.0f);
			//it->color = XMFLOAT4((temperature * tempScale) / 3000.0f, 0.0f, 1.0f - (temperature * tempScale) / 3000.0f, 1.0f);
			if (water)
			{
				col = Maths::AddFloat3(col, Maths::one);
				col = Maths::ScalarFloat3(col, 0.5f);
				col = Maths::ScalarFloat3(col, waterColor);

				//col.z += 0.25 * (waterColor - waterHeight);
				col.z += 0.5f;
				col = Maths::ScalarFloat3(col, 0.5f);
				//col = Blend(col, waterCol);
				//col = Blend(col, waterCol);
			}
			if (lava)
			{
				col.x *= waterColor;
				col.y *= waterColor;
				col.z *= waterColor * 0.25f;
				col.x += 0.5 * (waterColor - waterHeight);
				col.y += 0.25 * (waterColor - waterHeight);

				//it->color = XMFLOAT4(1.0f, 0.25f, 0.0f, 1.0f);
			}
			if (ice)
			{
				col = Maths::ScalarFloat3(col, 0.15f);
				col = Maths::AddFloat3(col, XMFLOAT3(0.55f, 0.55f, 0.55f));

				col.x *= waterColor;
				col.y *= waterColor;
				col.z *= waterColor;

				if (col.x > 1.0f) col.x = 1.0f;
				if (col.y > 1.0f) col.y = 1.0f;
				if (col.z > 1.0f) col.z = 1.0f;
				col.z += 0.45;
			}

			if (cancel)
			{
				//h_building = false;
				//h_finished++;
				//return;
			}

			if (h_map == NULL)
			{
				//h_building = false;
				//h_finished++;
				//return;
			}

			h_map[(z * h_res * h_res) + (y * h_res) + x] = (float)newValue;
			c_map[(z * h_res * h_res) + (y * h_res) + x] = col;
			//c_map[(z * h_res * h_res) + (y * h_res) + x] = XMFLOAT3((m_temp * tempScale) / 3000.0f, 0.0f, 1.0f - (m_temp * tempScale) / 3000.0f);

		}
	}

	delete[] data;
}

bool Mapping::Shutdown()
{
	if (h_finished == 6)
	{
		//h_built = true;
	}
	if (!h_built) return false;
	if (!h_built && h_building)
	{
		cancel = true;
		//return;
	}
	delete[] h_map;
	delete[] c_map;
	h_map = NULL;
	c_map = NULL;
	h_building = false;
	h_built = false;
	h_finished = 0;

	return true;
}

float Mapping::GetHeightMapValue(int face, int x, int y)
{
	if (x < 0) x = 0;
	if (x >= h_res) x = h_res;

	if (y < 0) y = 0;
	if (y >= h_res) y = h_res;

	return h_map[(face * h_res * h_res) + (y * h_res) + x];
}

float Mapping::GetHeightMapValueFloat(int face, float xCoordFloat, float yCoordFloat)
{
	int xCoord = (int)xCoordFloat;
	int yCoord = (int)yCoordFloat;

	float x = xCoordFloat - xCoord;
	float y = yCoordFloat - yCoord;

	//now i'm grabbing the points around it in order to average
	float a = GetHeightMapValue(face, (int)floor(xCoordFloat), (int)floor(yCoordFloat));
	float b = GetHeightMapValue(face, (int)ceil(xCoordFloat), (int)floor(yCoordFloat));
	float c = GetHeightMapValue(face, (int)floor(xCoordFloat), (int)ceil(yCoordFloat));
	float d = GetHeightMapValue(face, (int)ceil(xCoordFloat), (int)ceil(yCoordFloat));

	if (xCoord < 0) xCoord = 0;
	if (xCoord >= h_res) xCoord = h_res - 1;
	if (yCoord < 0) yCoord = 0;
	if (yCoord >= h_res) yCoord = h_res - 1;

	float xAvg = ((1.0f - y)*a + y*c) * (1.0f - x) + ((1.0f - y)*b + y*d) * x;
	float yAvg = ((1.0f - x)*a + x*b) * (1.0f - y) + ((1.0f - x)*c + x*d) * y;

	//float value = h_map[(faceDir * h_res * h_res) + (yCoord * h_res) + xCoord];
	float value = (xAvg + yAvg) / 2.0f;

	return value;
}

XMFLOAT3 Mapping::GetColorMapValue(int face, int x, int y)
{
	return c_map[(face * h_res * h_res) + (y * h_res) + x];
}

int Mapping::GetHeightMapRes()
{
	return h_res;
}

bool Mapping::IsBuilt()
{
	return h_built;
}

bool Mapping::IsBuilding()
{
	return h_building;
}

void Mapping::Cancel()
{
	cancel = true;
}

bool Mapping::Cancelled()
{
	return cancel;
}

void Mapping::SetPlanet(void* planet)
{
	m_planet = planet;
}

void* Mapping::CurrentPlanet()
{
	return m_planet;
}

XMFLOAT3 Mapping::Blend(XMFLOAT3 a, XMFLOAT3 b)
{

	XMFLOAT3 result;

	if (b.x < 0.5f)
	{
		result.x = (2.0f * a.x * b.x) + (a.x * a.x) * (1.0f - (2.0f * b.x));
	}
	else
	{
		result.x = (2.0f * a.x) * (1.0f - b.x) + sqrt(a.x) * ((2.0f * b.x) - 1.0f);
	}

	if (b.y < 0.5f)
	{
		result.y = (2.0f * a.y * b.y) + (a.y * a.y) * (1.0f - (2.0f * b.y));
	}
	else
	{
		result.y = (2.0f * a.y) * (1.0f - b.y) + sqrt(a.y) * ((2.0f * b.y) - 1.0f);
	}

	if (b.z < 0.5f)
	{
		result.x = (2.0f * a.z * b.z) + (a.z * a.z) * (1.0f - (2.0f * b.z));
	}
	else
	{
		result.x = (2.0f * a.z) * (1.0f - b.z) + sqrt(a.z) * ((2.0f * b.z) - 1.0f);
	}
	
	return result;
}

float Mapping::GetWaterHeight()
{
	return m_waterHeight;
}