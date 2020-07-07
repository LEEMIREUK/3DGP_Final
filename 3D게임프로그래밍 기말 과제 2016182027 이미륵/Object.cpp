//-----------------------------------------------------------------------------
// File: CGameObject.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Object.h"
#include "Shader.h"

CGameObject::CGameObject()
{
	random_device rd;
	mt19937 mt0(rd());
	uniform_int_distribution<> uid0;

	m_xmf4x4World = Matrix4x4::Identity();
	iMoveDir = uid0(mt0) % 6;
	fEnemySpeed = (float)(uid0(mt0) % 50 + 10);
}

CGameObject::~CGameObject()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pShader) m_pShader->Release();
}

void CGameObject::SetMesh(CMesh* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetShader(CShader* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject::Animate(float fTimeOver)
{
	if (bShootCheck == true)
		BulletMove(fTimeOver);
	if (bCubeCheck == true)
		Cube(fTimeOver);

	if (iEnemyCreate <= 0 && iEnemyCreate > -1)
	{
		SetPosition(XMFLOAT3(float(rand() % 180 - 90.0f), float(rand() % 40 + 18.0f), float(rand() % 350 - 200.0f)));
		iEnemyCreate = -1;
	}
	else if (iEnemyCreate > 0)
		iEnemyCreate -= fTimeOver;
}


void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMFLOAT4X4 xmf4x4World;
	xmf4x4World = Matrix4x4::Transpose(m_xmf4x4World);
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);

	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 3, &m_xmf3Color, 16);
}

void CGameObject::ReleaseShaderVariables()
{
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pMesh) m_pMesh->Render(pd3dCommandList);
}

void CGameObject::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	return(Vector3::Normalize((XMFLOAT3&)XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject::GetUp()
{
	return(Vector3::Normalize((XMFLOAT3&)XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject::GetRight()
{
	return(Vector3::Normalize((XMFLOAT3&)XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject::MoveLR(float fDistance)
{
	if (GetPosition().x > LEFT && GetPosition().x < RIGHT)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Right = GetRight();
		xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
		CGameObject::SetPosition(xmf3Position);
	}
}

void CGameObject::MoveUp(float fDistance)
{
	if (GetPosition().y > BOTTOM && GetPosition().y < TOP) 
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Up = GetUp();
		xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
		CGameObject::SetPosition(xmf3Position);
	}
}

void CGameObject::MoveForward(float fDistance)
{
	if (GetPosition().z > BACK && GetPosition().z < FORWARD) 
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMFLOAT3 xmf3Look = GetLook();
		xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
		CGameObject::SetPosition(xmf3Position);
	}
}

void CGameObject::MoveRandom(float fTimeOver)
{
	iChangeDir--;
	if (GetPosition().x < LEFT + 10.f || GetPosition().x > RIGHT - 10.f || GetPosition().y < BOTTOM + 10.f || GetPosition().y > TOP - 10.f ||
		GetPosition().z < BACK + 10.f || GetPosition().z > FORWARD - 10.f || iChangeDir == 0)
	{
		random_device rd;
		mt19937 mt0(rd());
		uniform_int_distribution<> uid0;
		uniform_int_distribution<> uid1{ 20, 80 };
		int gout = rand() % 10;
		if(gout%2)
			SetPosition(50.f - uid1(mt0), 100.f - uid1(mt0), 200.f - uid1(mt0));

		if (iChangeDir == 0)
			iChangeDir = rand() % 300 + 300;
		iMoveDir = uid0(mt0) % 26;
	}
	// -x
	if (iMoveDir == 0)
		MoveLR(-fEnemySpeed * fTimeOver);
	// x
	else if (iMoveDir == 1)
		MoveLR(+fEnemySpeed * fTimeOver);
	// -y
	else if (iMoveDir == 2)
		MoveUp(-fEnemySpeed * fTimeOver);
	// y	
	else if (iMoveDir == 3)
		MoveUp(+fEnemySpeed * fTimeOver);
	// -z
	else if (iMoveDir == 4)
		MoveForward(-fEnemySpeed * fTimeOver);
	// z
	else if (iMoveDir == 5)
		MoveForward(+fEnemySpeed * fTimeOver);
	// xy
	else if (iMoveDir == 6)
	{
		MoveLR(+fEnemySpeed * fTimeOver);
		MoveUp(+fEnemySpeed * fTimeOver);
	}
	// -xy
	else if (iMoveDir == 7)
	{
		MoveLR(-fEnemySpeed * fTimeOver);
		MoveUp(+fEnemySpeed * fTimeOver);
	}
	// x-y
	else if (iMoveDir == 8)
	{
		MoveLR(+fEnemySpeed * fTimeOver);
		MoveUp(-fEnemySpeed * fTimeOver);
	}
	// -x-y
	else if (iMoveDir == 9)
	{
		MoveLR(-fEnemySpeed * fTimeOver);
		MoveUp(-fEnemySpeed * fTimeOver);
	}
	// yz
	else if (iMoveDir == 10)
	{
		MoveUp(+fEnemySpeed * fTimeOver);
		MoveForward(+fEnemySpeed * fTimeOver);
	}
	// -yz
	else if (iMoveDir == 11)
	{
		MoveUp(-fEnemySpeed * fTimeOver);
		MoveForward(+fEnemySpeed * fTimeOver);
	}
	// y-z
	else if (iMoveDir == 12)
	{
		MoveUp(+fEnemySpeed * fTimeOver);
		MoveForward(-fEnemySpeed * fTimeOver);
	}
	// -y-z
	else if (iMoveDir == 13)
	{
		MoveUp(-fEnemySpeed * fTimeOver);
		MoveForward(-fEnemySpeed * fTimeOver);
	}
	// xz
	else if (iMoveDir == 14)
	{
		MoveLR(+fEnemySpeed * fTimeOver);
		MoveForward(+fEnemySpeed * fTimeOver);
	}
	// -xz
	else if (iMoveDir == 15)
	{
		MoveLR(-fEnemySpeed * fTimeOver);
		MoveForward(+fEnemySpeed * fTimeOver);
	}
	// x-z
	else if (iMoveDir == 16)
	{
		MoveLR(+fEnemySpeed * fTimeOver);
		MoveForward(-fEnemySpeed * fTimeOver);
	}
	// -x-z
	else if (iMoveDir == 17)
	{
		MoveLR(-fEnemySpeed * fTimeOver);
		MoveForward(-fEnemySpeed * fTimeOver);
	}
	// xyz
	else if (iMoveDir == 18)
	{
		MoveLR(+fEnemySpeed * fTimeOver);
		MoveUp(+fEnemySpeed * fTimeOver);
		MoveForward(+fEnemySpeed * fTimeOver);
	}
	// -xyz
	else if (iMoveDir == 19)
	{
		MoveLR(-fEnemySpeed * fTimeOver);
		MoveUp(+fEnemySpeed * fTimeOver);
		MoveForward(+fEnemySpeed * fTimeOver);
	}
	// x-yz
	else if (iMoveDir == 20)
	{
		MoveLR(+fEnemySpeed * fTimeOver);
		MoveUp(-fEnemySpeed * fTimeOver);
		MoveForward(+fEnemySpeed * fTimeOver);
	}
	// xy-z
	else if (iMoveDir == 21)
	{
		MoveLR(+fEnemySpeed * fTimeOver);
		MoveUp(+fEnemySpeed * fTimeOver);
		MoveForward(-fEnemySpeed * fTimeOver);
	}
	// -x-yz
	else if (iMoveDir == 22)
	{
		MoveLR(-fEnemySpeed * fTimeOver);
		MoveUp(-fEnemySpeed * fTimeOver);
		MoveForward(+fEnemySpeed * fTimeOver);
	}
	// -xy-z
	else if (iMoveDir == 23)
	{
		MoveLR(-fEnemySpeed * fTimeOver);
		MoveUp(+fEnemySpeed * fTimeOver);
		MoveForward(-fEnemySpeed * fTimeOver);
	}
	// x-y-z
	else if (iMoveDir == 24)
	{
		MoveLR(+fEnemySpeed * fTimeOver);
		MoveUp(-fEnemySpeed * fTimeOver);
		MoveForward(-fEnemySpeed * fTimeOver);
	}
	// -x-y-z
	else if (iMoveDir == 25)
	{
		MoveLR(-fEnemySpeed * fTimeOver);
		MoveUp(-fEnemySpeed * fTimeOver);
		MoveForward(-fEnemySpeed * fTimeOver);
	}
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::BulletMove(float fTimeOver)
{
	XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
	XMFLOAT3 m_xmf3Position{ m_xmf4x4World._41, m_xmf4x4World._42 ,m_xmf4x4World._43 };
	xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3BulletLook, fBulletSpeed);
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
	SetPosition(m_xmf3Position);

	if (GetPosition().x < LEFT - 100.f || GetPosition().x > RIGHT + 100.f || GetPosition().y < 0 || GetPosition().y > 100 ||
		GetPosition().z < BACK - 100.f || GetPosition().z > FORWARD + 100.f)
	{
		SetPosition(0.f, -10.f, 0.f);
		bShootCheck = false;
	}
}
void CGameObject::Cube(float fTimeOver)
{
	if (fEffectTime > 0)
	{
		float fDiagonal = sqrt(fEffectSpeed * fEffectSpeed + fEffectSpeed * fEffectSpeed) * fTimeOver;
		float Speed = fTimeOver * fEffectSpeed;
		if (iCubeNum == 0)
			MoveForward(Speed);
		else if (iCubeNum == 1)
			MoveForward(-Speed);
		else if (iCubeNum == 2)
			MoveUp(Speed);
		else if (iCubeNum == 3)
			MoveUp(-Speed);
		else if (iCubeNum == 4)
			MoveLR(Speed);
		else if (iCubeNum == 5)
			MoveLR(-Speed);
		else if (iCubeNum == 6)
		{
			MoveForward(fDiagonal);
			MoveUp(fDiagonal);
		}
		else if (iCubeNum == 7)
		{
			MoveForward(fDiagonal);
			MoveUp(-fDiagonal);
		}
		else if (iCubeNum == 8)
		{
			MoveForward(fDiagonal);
			MoveLR(fDiagonal);
		}
		else if (iCubeNum == 9)
		{
			MoveForward(fDiagonal);
			MoveLR(-fDiagonal);
		}
		else if (iCubeNum == 10)
		{
			MoveUp(fDiagonal);
			MoveLR(fDiagonal);
		}
		else if (iCubeNum == 11)
		{
			MoveUp(fDiagonal);
			MoveLR(-fDiagonal);
		}
		else if (iCubeNum == 12)
		{
			MoveForward(-fDiagonal);
			MoveUp(fDiagonal);
		}
		else if (iCubeNum == 13)
		{
			MoveForward(-fDiagonal);
			MoveUp(-fDiagonal);
		}
		else if (iCubeNum == 14)
		{
			MoveForward(-fDiagonal);
			MoveLR(fDiagonal);
		}
		else if (iCubeNum == 15)
		{
			MoveForward(-fDiagonal);
			MoveLR(-fDiagonal);
		}
		else if (iCubeNum == 16)
		{
			MoveUp(-fDiagonal);
			MoveLR(fDiagonal);
		}
		else if (iCubeNum == 17)
		{
			MoveUp(-fDiagonal);
			MoveLR(-fDiagonal);
		}
		else if (iCubeNum == 18)
			MoveForward(Speed * 2);
		else if (iCubeNum == 19)
			MoveForward(-Speed * 2);
		else if (iCubeNum == 20)
			MoveUp(Speed * 2);
		else if (iCubeNum == 21)
			MoveUp(-Speed * 2);
		else if (iCubeNum == 22)
			MoveLR(Speed * 2);
		else if (iCubeNum == 23)
			MoveLR(-Speed * 2);

		fEffectTime -= fTimeOver;
	}
	else 
		SetPosition(0.f, -10.f, 0.f);
}

CUfoObject::CUfoObject()
{
}
CUfoObject::~CUfoObject()
{
}

void CGameObject::Reset()
{
	if (m_pMesh) m_pMesh->Release();
	if (m_pShader) m_pShader->Release();

	random_device rd;
	mt19937 mt0(rd());
	uniform_int_distribution<> uid0;

	m_xmf4x4World = Matrix4x4::Identity();
	iMoveDir = uid0(mt0) % 6;
	fEnemySpeed = (uid0(mt0) % 6 + 1) / (float)70;
}