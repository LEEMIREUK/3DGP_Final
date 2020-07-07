#pragma once

#include "Mesh.h"
#include "Camera.h"

class CShader;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

public:
	bool bBulletCheck = false;
	bool bShootCheck = false;
	bool bCubeCheck = false;

	char m_pstrName[64] = { '\0' };

	int iMoveDir;
	int iEnemyCreate = -1;
	int iChangeDir = 0;
	int iCubeNum = 0;
	float fEnemySpeed;
	float fBulletSpeed = 0.5f;

	float fEffectTime = 0.f;
	float fEffectSpeed = 5.f;

	XMFLOAT3 m_xmf3BulletLook = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 m_xmf3BulletRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 m_xmf3BulletUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	BoundingBox aabb;

	XMFLOAT4X4						m_xmf4x4World;
	CMesh* m_pMesh = NULL;

	CShader* m_pShader = NULL;

	XMFLOAT3						m_xmf3Color = XMFLOAT3(1.0f, 1.0f, 1.0f);

	void SetMesh(CMesh* pMesh);
	void SetShader(CShader* pShader);
	void SetWorldMatrix(XMFLOAT4X4& xmf4x4World) { m_xmf4x4World = xmf4x4World; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void Animate(float fTimeOver);
	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	virtual void ReleaseUploadBuffers();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

	void SetColor(XMFLOAT3 xmf3Color) { m_xmf3Color = xmf3Color; }

	void MoveLR(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void MoveRandom(float fTimeOver);
	void BulletMove(float fTimeOver);
	void Cube(float fTimeOver);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Reset();
};

class CUfoObject : public CGameObject
{
public:
	CUfoObject();
	virtual ~CUfoObject();
};