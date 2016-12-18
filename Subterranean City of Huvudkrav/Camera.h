#pragma once
#include "Resource.h"
class Camera
{
private:
	XMFLOAT3	pos,
				dir,
				up;
	float pitchRotation;
	void updateCBuffers();

public:

	Camera(float x, float y, float z);

	void		move();
	void		fly(XMFLOAT3 movement);
	void		rotate(float yaw, float pitch, float roll);
	XMFLOAT3	getPos() { return pos; }
	XMFLOAT3	getDir() { return dir; }
	XMFLOAT3	getUp() { return up; }
	~Camera();
};

