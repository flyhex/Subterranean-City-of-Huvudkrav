#include "Object3D.h"

Object3D::Object3D(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
{
	this->pos	= pos;
	this->rot	= rot;
	this->scale	= scale;
	updateMWorld();
}
void Object3D::updateMWorld()
{
	//change to ownmade
	XMStoreFloat4x4(&mScale, XMMatrixScalingFromVector( XMLoadFloat3(&scale) ) );
	XMStoreFloat4x4(&mRot, XMMatrixRotationRollPitchYawFromVector( XMLoadFloat3(&rot) ) );
	XMStoreFloat4x4(&mTrans, XMMatrixTranslationFromVector( XMLoadFloat3(&pos) ) );

	XMStoreFloat4x4(&mWorld, XMMatrixMultiply(XMLoadFloat4x4(&mScale),XMLoadFloat4x4(&mRot) ) );
	XMStoreFloat4x4(&mWorld, XMMatrixMultiply(XMLoadFloat4x4(&mWorld),XMLoadFloat4x4(&mTrans) ) );
}
void Object3D::updateCbWorld()
{
	GraphicsCore *g = GraphicsCore::getInstance();
	cbWorld.world = XMLoadFloat4x4( &mWorld );
	g->immediateContext->UpdateSubresource( g->cbWorld, 0, NULL, &cbWorld, 0, 0 );
}

void Object3D::rotate( float x, float y, float z )
{
	rot.x += x;
	rot.y += y;
	rot.z += z;
	updateMWorld();
}

Object3D::~Object3D()
{

}
