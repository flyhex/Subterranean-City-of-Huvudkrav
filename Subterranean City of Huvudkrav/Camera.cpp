#include "Camera.h"
#include "GraphicsCore.h"

Camera::Camera(float x, float y, float z)
{
	pitchRotation	=	0;
	//maybe not the best place to have the view matrix for the light which has shadowmap
	XMMATRIX mProj, mProjInv, mLightView;
	GraphicsCore *g =  GraphicsCore::getInstance();

	mProj = XMMatrixPerspectiveFovLH(XM_PI * 0.4f,
												(float)DEFAULTSCRNWIDTH/(float)DEFAULTSCRNHEIGHT,
												0.1f, 8000.0f);
	mProjInv = XMMatrixInverse( &XMMatrixDeterminant( mProj ), mProj );
	XMStoreFloat3( &pos, XMVectorSet(x,y,z,1.0f) );
	XMStoreFloat3( &dir, XMVector3Normalize( XMVectorSet(0,0,1,0.0f) ) );
	XMStoreFloat3( &up, XMVectorSet( .0f, 1.0f, .0f, 0.0f ) );

	XMVECTOR lightViewPos = XMVectorSet(-10,2000,4800,1);
	//XMVECTOR lightViewPos = XMVectorSet(-400,1500,6000,1);
	XMVECTOR lightViewDir = -lightViewPos;
	mLightView = XMMatrixLookAtLH(lightViewPos,XMVectorSet(4000,0,0,1),XMVectorSet(0,1,0,0));

	

	CBOnce cb;
	cb.projection	= mProj;
	cb.projInv		= mProjInv;
	cb.lightView	= mLightView;
	g->immediateContext->UpdateSubresource( g->cbOnce, 0, NULL, &cb, 0, 0 );
}
void Camera::updateCBuffers()
{
	XMMATRIX	mView, mViewInv, mViewRefl;
	XMVECTOR	xPos = XMLoadFloat3(&pos);
	XMVECTOR	xDir = XMLoadFloat3(&dir); 
	GraphicsCore *g =  GraphicsCore::getInstance();

	mView = XMMatrixLookAtLH(xPos,XMVectorAdd(xPos,XMLoadFloat3(&dir)),XMLoadFloat3(&up) );
	mViewInv = XMMatrixInverse( &XMMatrixDeterminant( mView ), mView);

	//should be collected from water
	float waterLevel = 0.0f;
	XMFLOAT3 reflPos, reflDir;
	reflPos = pos;
	reflPos.y = waterLevel - (reflPos.y - waterLevel);

	reflDir = dir;
	reflDir.y = waterLevel - (reflDir.y - waterLevel);

	mViewRefl = XMMatrixLookAtLH(XMLoadFloat3(&reflPos),XMVectorAdd(XMLoadFloat3(&reflPos),XMLoadFloat3(&reflDir)),XMVectorSet(0,-1,0,0 ));
	CBReflection cbR;
	cbR.viewRefl = mViewRefl;

	CBCameraMove cb;
	cb.view = mView;
	cb.viewInv = mViewInv;
	//cb.viewRefl = mViewRefl;
	cb.cameraPos = xPos;
	cb.cameraDir = xDir;
	
	g->immediateContext->UpdateSubresource( g->cbCameraMove, 0, NULL, &cb, 0, 0 );
	g->immediateContext->UpdateSubresource( g->cbReflection, 0, NULL, &cbR, 0, 0 );
}

void Camera::move()
{

}

void Camera::fly(XMFLOAT3 movement)
{
	XMVECTOR xDir = XMLoadFloat3(&dir);
	XMVECTOR xUp = XMLoadFloat3(&up);
	XMVECTOR strafe = XMVector3Cross( XMLoadFloat3(&dir), XMLoadFloat3(&up) );

	XMVECTOR newpos = XMLoadFloat3(&pos) +
						strafe * movement.x*1000 +
						xDir * movement.z*1000 +
						XMLoadFloat3(&XMFLOAT3(0, movement.y*1000,0) );
	XMStoreFloat3(&pos, newpos);

	updateCBuffers();
}
void Camera::rotate(float yaw, float pitch, float roll)
{
	XMMATRIX mRotation;
	XMVECTOR xDir = XMLoadFloat3(&dir);
	XMVECTOR xUp = XMLoadFloat3(&up);
	XMVECTOR rotationAxis;

	if(pitchRotation - pitch/XMConvertToRadians( 180) > XMConvertToRadians(270))
		pitch = max(pitchRotation,XMConvertToRadians(270)) - min(pitchRotation,XMConvertToRadians(270));
	else if(pitchRotation - pitch/XMConvertToRadians(180) < -XMConvertToRadians(270))
		pitch = min(pitchRotation,-XMConvertToRadians(270)) - max(pitchRotation,-XMConvertToRadians(270));
	pitchRotation -= pitch;
	
	rotationAxis = XMVector3Cross(xDir, xUp);
	rotationAxis = XMVector3Normalize(rotationAxis);

	mRotation = XMMatrixRotationAxis(rotationAxis, -pitch) * XMMatrixRotationY(yaw) * XMMatrixRotationZ(roll);

	xDir = XMVector3TransformCoord(xDir, mRotation);
	xDir = XMVector3Normalize(xDir);
	XMStoreFloat3(&dir, xDir);

	xUp = XMVector3TransformCoord(xUp, mRotation);
	xUp = XMVector3Normalize(xUp);
	XMStoreFloat3(&up, xUp);

	updateCBuffers();
}
Camera::~Camera()
{

}
