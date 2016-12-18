#include "HeightMap.h"
#include <iostream>

HeightMap::HeightMap()
{
}
void HeightMap::loadRaw(int width, int height, const string& filename, float heightScale, float heightOffset)
{
	vector<unsigned char> pointHeights(width*height);
	ifstream fin;
	fin.open(filename.c_str(), ios_base::binary);
	if(!fin)
	{
		cout << "could not open file: " << filename.c_str(); //throw exception("error loading heightmap");
		cout << ". Random heights added" << endl;
		for (int i = 0; i < width * height; i++)
		{
			heights.push_back(rand()%100);
		}
		boxFilter(width, height);
		return;
	}

	fin.read((char*)&pointHeights[0], (streamsize)pointHeights.size());
	fin.close();

	for(int i = 0; i < width * height; i++)
		heights.push_back((float)pointHeights[i] * heightScale + heightOffset);
	boxFilter(width, height);
}
void HeightMap::boxFilter(int width, int height)
{
	vector<float> temp(heights.size());
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width;j++)
		{
			temp[i * width + j] = sampleBoxFilter(width,height,i,j);
		}
	}
	heights = temp;
}
float HeightMap::sampleBoxFilter(int width, int height, int i, int j)
{
	float tot = 0.0f;
	float num = 0.0f;
	for( int m = i - 1; m <= i + 1; m++)
	{
		for( int n = j - 1; n <= j + 1; n++)
		{
			if( m >= 0 && m < height && n >= 0 && n < width)
			{
				tot += heights[m * width + n];
				num += 1.0f;
			}
		}
	}
	return tot/num;
}
HeightMap::~HeightMap()
{
}
