//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::LoadSceneObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	CPseudoLightingShader* pShader = new CPseudoLightingShader();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature);
	pShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);

	char pstrToken[64] = { '\0' };
	char pstrGameObjectName[64] = { '\0' };

	UINT nReads = 0, nObjectNameLength = 0;
	BYTE nStrLength = 0;

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), 14, pFile); 
	nReads = (UINT)::fread(&m_iObject, sizeof(int), 1, pFile);

	m_iObject += m_iEnemy;
	m_ppObjects = new CGameObject * [m_iObject + m_iBulletNum + iCube];

	CGameObject* pGameObject = NULL;
	for (int i = 0; i < m_iObject; i++)
	{
		pGameObject = new CGameObject();

		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), 13, pFile);
		nReads = (UINT)::fread(&nObjectNameLength, sizeof(UINT), 1, pFile);
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
		nReads = (UINT)::fread(pstrGameObjectName, sizeof(char), nObjectNameLength, pFile);
		nReads = (UINT)::fread(&pGameObject->m_xmf4x4World, sizeof(float), 16, pFile);

		pstrGameObjectName[nObjectNameLength] = '\0';
		strcpy_s(pGameObject->m_pstrName, 64, pstrGameObjectName);

		CMesh* pMesh = NULL;
		for (int j = 0; j < i; j++)
		{
			if (!strcmp(pstrGameObjectName, m_ppObjects[j]->m_pstrName))
			{
				pMesh = m_ppObjects[j]->m_pMesh;
				break;
			}
		}
		if (!pMesh)
		{
			char pstrFilePath[64] = { '\0' };
			strcpy_s(pstrFilePath, 64, "Models/");
			strcpy_s(pstrFilePath + 7, 64 - 7, pstrGameObjectName);
			strcpy_s(pstrFilePath + 7 + nObjectNameLength, 64 - 7 - nObjectNameLength, ".bin");
			pMesh = new CMesh(pd3dDevice, pd3dCommandList, pstrFilePath);
		}

		pGameObject->SetMesh(pMesh);
		pGameObject->SetShader(pShader);

		m_ppObjects[i] = pGameObject;
	}

	::fclose(pFile);

	CMesh* pUfoMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/UFO.txt");
	CMesh* pFlyerMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/FlyerPlayership.txt");

	for (int i = 2; i < 52; ++i)
	{
		random_device rd;
		mt19937 mt0(rd());
		uniform_int_distribution<> uid0{20, 80};

		mt19937 mt1(rd());
		uniform_real_distribution<> urd0(0, 1);
		float r = urd0(rd);
		float g = urd0(rd);
		float b = urd0(rd);
		m_ppObjects[m_iObject - i] = new CGameObject();
		m_ppObjects[m_iObject - i]->SetMesh(pUfoMesh);
		m_ppObjects[m_iObject - i]->SetShader(pShader);
		m_ppObjects[m_iObject - i]->SetPosition(50.f - uid0(mt0), 100.f - uid0(mt0), 200.f - uid0(mt0));
		m_ppObjects[m_iObject - i]->SetColor(XMFLOAT3(r, g, b));
	}
	
	//Cube
	CMesh* pCubeMesh = new CMesh(pd3dDevice, pd3dCommandList, "Models/Cube.txt");

	//bullet
	for (int i = m_iObject - 1; i < m_iObject + m_iBulletNum; i++) {
		m_ppObjects[i] = new CGameObject();
		m_ppObjects[i]->bBulletCheck = true;
		m_ppObjects[i]->SetMesh(pCubeMesh);
		m_ppObjects[i]->SetColor(XMFLOAT3(1.f, 1.f, 0.f));
		m_ppObjects[i]->SetPosition(0.f, -10.f, 0.f);
	}

	// death effect
	for (int i = m_iObject + m_iBulletNum - 1; i < m_iObject + m_iBulletNum + iCube; i++) {
		m_ppObjects[i] = new CGameObject();
		m_ppObjects[i]->bCubeCheck = true;
		m_ppObjects[i]->iCubeNum = i - m_iObject - m_iBulletNum + 1;
		m_ppObjects[i]->SetMesh(pCubeMesh);
		m_ppObjects[i]->SetColor(XMFLOAT3(1.f, 0.f, 0.f));
		m_ppObjects[i]->SetPosition(0.f, -10.f, 0.f);
	}
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	LoadSceneObjectsFromFile(pd3dDevice, pd3dCommandList, (char*)"Models/Scene.bin");
}

void CScene::BuildBullet(CPlayer& pPlayer, int iChooseBullet)
{
	m_ppObjects[m_iObject + iChooseBullet]->SetPosition(pPlayer.GetPosition());
	m_ppObjects[m_iObject + iChooseBullet]->m_xmf4x4World = pPlayer.m_xmf4x4World;
	m_ppObjects[m_iObject + iChooseBullet]->m_xmf3BulletLook = pPlayer.GetLookVector();
	m_ppObjects[m_iObject + iChooseBullet]->m_xmf3BulletUp = pPlayer.GetUpVector();
	m_ppObjects[m_iObject + iChooseBullet]->m_xmf3BulletRight = pPlayer.GetRightVector();
	m_ppObjects[m_iObject + iChooseBullet]->bShootCheck = true;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[3];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[0].Constants.Num32BitValues = 4;
	pd3dRootParameters[0].Constants.ShaderRegister = 0;
	pd3dRootParameters[0].Constants.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 19;
	pd3dRootParameters[1].Constants.ShaderRegister = 1; 
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[2].Constants.Num32BitValues = 35;
	pd3dRootParameters[2].Constants.ShaderRegister = 2;
	pd3dRootParameters[2].Constants.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_ppObjects)
	{
		for (int j = 0; j < m_iObject; ++j)
			if (m_ppObjects[j])
				delete m_ppObjects[j];
		delete[] m_ppObjects;
	}
}

void CScene::ReleaseUploadBuffers()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_iObject; ++j)
			if (m_ppObjects[j]) m_ppObjects[j]->ReleaseUploadBuffers();
	}
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput()
{
	return(false);
}

void CScene::AnimateObjects(float fTimeOver)
{
	for (int j = m_iObject - m_iEnemy - 1; j < m_iObject; ++j)
		m_ppObjects[j]->Animate(fTimeOver);

	for (int j = m_iObject - m_iEnemy - 1; j < m_iObject - 1; ++j)
	{
		if (m_ppObjects[j])
		{
			m_ppObjects[j]->Animate(fTimeOver);
			m_ppObjects[j]->MoveRandom(fTimeOver);
			m_ppObjects[j]->aabb = BoundingBox(m_ppObjects[j]->GetPosition(), XMFLOAT3(3.f, 2.f, 3.f));
		}
	}

	for (int i = m_iObject - 1; i < m_iObject + m_iBulletNum; ++i)
	{
		m_ppObjects[i]->Animate(fTimeOver);
		m_ppObjects[i]->aabb = BoundingBox(m_ppObjects[i]->GetPosition(), XMFLOAT3(1.f, 1.f, 1.f));
	}

	for (int i = m_iObject + m_iBulletNum - 1; i < m_iObject + m_iBulletNum + iCube; ++i)
	{
		m_ppObjects[i]->Animate(fTimeOver);
	}

	for (int i = m_iObject - 1; i < m_iObject + m_iBulletNum; ++i)
	{
		m_ppObjects[i]->Animate(fTimeOver);
		m_ppObjects[i]->aabb = BoundingBox(m_ppObjects[i]->GetPosition(), XMFLOAT3(1.0f, 1.0f, 1.0f));

		if (m_ppObjects[i]->bShootCheck == true)
		{
			for (int j = m_iObject - m_iEnemy - 1; j < m_iObject - 1; ++j) 
			{
				if (m_ppObjects[i]->aabb.Intersects(m_ppObjects[j]->aabb))
				{
					m_ppObjects[i]->SetPosition(0.f, -10.f, 0.f);
					m_ppObjects[i]->bShootCheck = false;
					for (int a = m_iObject + m_iBulletNum - 1; a < m_iObject + m_iBulletNum + iCube; ++a)
					{
						m_ppObjects[a]->SetPosition(m_ppObjects[j]->GetPosition());
						m_ppObjects[a]->fEffectTime = 0.5f;
					}
					m_ppObjects[j]->SetPosition(0.f, -1000.f, 0.f);
					m_ppObjects[j]->iEnemyCreate = 5;

					m_iScore += 10;
					if (m_iScore > 300)
						exit(1);
					break;
				}
			}
		}
	}
	for (int i = m_iObject + m_iBulletNum - 1; i < m_iObject + m_iBulletNum + iCube; ++i)
		m_ppObjects[i]->Animate(fTimeOver);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	for (int j = 0; j < m_iObject + m_iBulletNum + iCube; ++j)
	{
		if (m_ppObjects[j]) m_ppObjects[j]->Render(pd3dCommandList, pCamera);
	}
}

