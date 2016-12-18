#pragma once
#include "Resource.h"
#include "GraphicsCore.h"

class Object3D
{
private:
	XMFLOAT4X4	mScale,
				mTrans,
				mRot,
				mWorld,
				mWorldInv;
	CBWorld		cbWorld;
protected:
	XMFLOAT3	pos, rot, scale;
	XMFLOAT4X4	getWorld()					{ return mWorld; }
	XMFLOAT4X4	getWorldInv()				{ return mWorldInv; }

	void		updateMWorld();

public:
	Object3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale);
	void		updateCbWorld();
	void		rotate( float x, float y, float z );
	~Object3D();
};

