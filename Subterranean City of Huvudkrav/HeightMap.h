#pragma once
#include <fstream>
#include "resource.h"
class HeightMap
{
private:
	vector<float> heights;
	void boxFilter(int width, int height);
	float sampleBoxFilter(int width, int height, int i, int j);
public:
	HeightMap();
	~HeightMap();

	void loadRaw(int width, int height, const string& filename, float heightScale, float heightOffset);
	vector<float> getHeights() { return heights; }
};
