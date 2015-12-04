#include "GameObjectManager.h"
#include "DirectInput.h"
#include "SkinnedMesh.h"

GameObjectManager* m_pGameObjManager = nullptr;

/////////////////////////////////////////////////////////////////////////

GameObjectManager::GameObjectManager(bool bShouldRenderTitles, bool bShouldHighlightPickedObjects, bool bShouldRenderAxis, bool bAreObjectsGrounded, bool bShouldRenderBoundingBoxes)
{
	m_pPickedObject	= nullptr;
	m_bShouldHighlightPickedObjects = bShouldHighlightPickedObjects;
	m_bShouldRenderTitles  = bShouldRenderTitles;
	m_bShouldRenderAxis    = bShouldRenderAxis;
	m_bAreObjectsGrounded = bAreObjectsGrounded;
	m_bShouldRenderBoundingBoxes = bShouldRenderBoundingBoxes;
}

/////////////////////////////////////////////////////////////////////////

/*
Function:addGameObject
Purpose:puts new GameObject in the map
*/
void GameObjectManager::AddGameObject(GameObject* pGameObject)
{
	m_gameObjects.push_back(pGameObject);
}

/////////////////////////////////////////////////////////////////////////

GameObject* GameObjectManager::GetPickedObject()
{
	return m_pPickedObject;
}

/////////////////////////////////////////////////////////////////////////

void GameObjectManager::SetPickedObject(GameObject* pPickedObject)
{
	m_pPickedObject = pPickedObject;
}

/////////////////////////////////////////////////////////////////////////

void GameObjectManager::OnUpdate()
{
	for(auto& gameObject : m_gameObjects)
	{
		if(gameObject->GetObjectType() == EGameObjectType_Skinned )
		{	
			if( pDinput->IsMouseButtonDown(0) )
			{
				D3DXVECTOR3 vOrigin(0.0f, 0.0f, 0.0f);
				D3DXVECTOR3 vDir(0.0f, 0.0f, 0.0f);

				GetWorldPickingRay(vOrigin, vDir);
				
				AABB box = gameObject->GetBB().TransformByMatrix(gameObject->GetCombinedTransfMatrix());
				if( D3DXBoxBoundProbe(&box.GetMinPoint(), &box.GetMaxPoint(), &vOrigin, &vDir) )
				{
					SkinnedMesh* pMesh = static_cast<SkinnedMesh*>(gameObject);

					D3DXFRAME* pFrame = pMesh->FindFrameWithMesh(gameObject->GetRootFrame());

					float nDistance = 0.0f;

					//m_mapPickedObjects[nDistance] = it->second;

					if( IsPickedSkinnedObject(pFrame, gameObject->GetCombinedTransfMatrix(),vOrigin,vDir,nDistance) )
					{
						m_mapPickedObjects[nDistance] = gameObject;
					}
				}
			}
		}
		else if(gameObject->GetObjectType() == EGameObjectType_Static )
		{
			//if( pDinput->IsMouseButtonDown(0) )
			{
				float nDistance = 0.0f;
				if( IsPickedStaticObject(gameObject,nDistance) )
				{
					m_mapPickedObjects[nDistance] = gameObject;
				}
			}
		}

	}

	//the closest object is picked
	if( !m_mapPickedObjects.empty() )
	{
		auto pClosestPickedObject = (m_mapPickedObjects.begin()->second);
		if( pClosestPickedObject )
		{
			pClosestPickedObject->SetPicked(true);
		}

		m_mapPickedObjects.clear();
	}
}

/////////////////////////////////////////////////////////////////////////

bool GameObjectManager::IsPickedStaticObject(GameObject* pObj, float& nDistance)
{
	D3DXVECTOR3 vOrigin(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vDir(0.0f, 0.0f, 0.0f);

	GetWorldPickingRay(vOrigin, vDir);

	bool bIsPicked = false;

	AABB box = pObj->GetBB().TransformByMatrix(pObj->GetCombinedTransfMatrix());
	bIsPicked = D3DXBoxBoundProbe(&box.GetMinPoint(), &box.GetMaxPoint(), &vOrigin, &vDir);

	if( bIsPicked )
	{
		//by finding the inverse of mesh CombinedMatrix we transform from World space to the mesh local space
		//this is needed, because we have to transform the picking vectors to the mesh local space
		//which is required by the D3DXIntersect function
		D3DXMATRIX InverseWorldMatrix;
		D3DXMatrixInverse(&InverseWorldMatrix, 0, &pObj->GetCombinedTransfMatrix());

		//transform the Ray using the inverse matrix
		D3DXVec3TransformCoord(&vOrigin, &vOrigin, &InverseWorldMatrix);
		D3DXVec3TransformNormal(&vDir, &vDir, &InverseWorldMatrix);

		BOOL hit = false;
		DWORD faceIndex = -1;
		float u = 0.0f;
		float v = 0.0f;
		float dist = 0.0f;
		ID3DXBuffer* allhits = 0;
		DWORD numHits = 0;
		D3DXIntersect(pObj->GetMesh(), &vOrigin, &vDir, &hit,&faceIndex, &u, &v, &dist, &allhits, &numHits);
		releaseX(allhits);

		nDistance = dist;
		return hit;
	}
}

/////////////////////////////////////////////////////////////////////////

bool GameObjectManager::IsPickedSkinnedObject(D3DXFRAME* pFrame,D3DXMATRIX combinedMatrix,D3DXVECTOR3 vOrigin,D3DXVECTOR3 vDir, float& nDistance )
{
	D3DXMESHCONTAINER* pMeshContainer = pFrame->pMeshContainer;

	D3DXFRAME* pSibling	 = pFrame->pFrameSibling;
    D3DXFRAME* pFirstChild = pFrame->pFrameFirstChild;

	if( pMeshContainer != NULL )
	{
		//fout<<pFrame->Name << endl;

		D3DXMATRIX InverseWorldMatrix;
		D3DXMatrixInverse(&InverseWorldMatrix, 0, &combinedMatrix);

		//transform the Ray using the inverse matrix
		D3DXVec3TransformCoord(&vOrigin, &vOrigin, &InverseWorldMatrix);
		D3DXVec3TransformNormal(&vDir, &vDir, &InverseWorldMatrix);

		BOOL hit = false;
		DWORD faceIndex = -1;
		float u = 0.0f;
		float v = 0.0f;
		float dist = 0.0f;
		ID3DXBuffer* allhits = 0;
		DWORD numHits = 0;
	
		D3DXIntersect(pMeshContainer->MeshData.pMesh, &vOrigin, &vDir, &hit,&faceIndex, &u, &v, &dist, &allhits, &numHits);
		releaseX(allhits);

		nDistance = dist;

		return hit;
	}

	if( pSibling )
	{
		IsPickedSkinnedObject(pSibling, combinedMatrix,vOrigin,vDir,nDistance);
	}

	if( pFirstChild )
	{
		IsPickedSkinnedObject(pFirstChild, combinedMatrix,vOrigin,vDir,nDistance);
	}

	return false;
	
}

/////////////////////////////////////////////////////////////////////////

std::vector<GameObject*>& GameObjectManager::GetGameObjects()
{
	return m_gameObjects;
}

/////////////////////////////////////////////////////////////////////////

GameObject* GameObjectManager::GetObjectByName(std::string name)
{
	for(auto& object : m_gameObjects )
	{
		if( !object->GetName().compare(name) )
		{
			return object;
		}
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////

void GameObjectManager::SetShouldRenderTitles(bool bShouldRenderTitles)
{
	m_bShouldRenderTitles = bShouldRenderTitles;
}

/////////////////////////////////////////////////////////////////////////

bool GameObjectManager::ShouldRenderTitles()
{
	return m_bShouldRenderTitles;
}

/////////////////////////////////////////////////////////////////////////

void GameObjectManager::SetShouldHighlightPickedObjects(bool bShouldHighlightPickedObjects)
{
	m_bShouldHighlightPickedObjects = bShouldHighlightPickedObjects;
}

/////////////////////////////////////////////////////////////////////////

bool GameObjectManager::ShouldHighlightPickedObjects()
{
	return m_bShouldHighlightPickedObjects;
}

/////////////////////////////////////////////////////////////////////////

void GameObjectManager::SetShouldRenderAxis(bool bShouldRenderAxis)
{
	m_bShouldRenderAxis = bShouldRenderAxis;
}

/////////////////////////////////////////////////////////////////////////

bool GameObjectManager::ShouldRenderAxis()
{
	return m_bShouldRenderAxis;
}

/////////////////////////////////////////////////////////////////////////

void GameObjectManager::SetAreObjectsGrounded(bool bAreObjectsGrounded)
{
	m_bAreObjectsGrounded = bAreObjectsGrounded;
}

/////////////////////////////////////////////////////////////////////////

bool GameObjectManager::AreObjectsGrounded()
{
	return m_bAreObjectsGrounded;
}

/////////////////////////////////////////////////////////////////////////

void GameObjectManager::SetShouldRenderBoundingBoxes(bool bShouldRenderBoundingBoxes)
{
	m_bShouldRenderBoundingBoxes = bShouldRenderBoundingBoxes;
}

/////////////////////////////////////////////////////////////////////////

bool GameObjectManager::ShouldRenderBoundingBoxes()
{
	return m_bShouldRenderBoundingBoxes;
}

/////////////////////////////////////////////////////////////////////////