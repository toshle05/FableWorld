#include <stdafx.h>
#include "StaticModel.h"

/////////////////////////////////////////////////////////////////////////

StaticModel::StaticModel() {
	//default texture for models that dont have any
	CheckSuccess(D3DXCreateTextureFromFile(pApp->GetDevice(), "../../Resources/textures/DefaultWhiteTexture.dds", &m_pWhiteTexture));

	m_light.m_vLight   = D3DXVECTOR3(20.0f, 300.0f, 50.0f);
	m_light.m_ambient  = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	m_light.m_diffuse  = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	m_light.m_specular = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	D3DXVec3Normalize(&m_light.m_vLight, &m_light.m_vLight);

	m_bIsPicked = false;

	BuildEffect();

	m_vLook	 = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_vRight = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	m_vUp	 = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////////

StaticModel::StaticModel(std::string strModelName, std::string ModelFileName, std::string strTextureFileName) {
	//default texture for models that dont have any
	CheckSuccess(D3DXCreateTextureFromFile(pApp->GetDevice(), "../../Resources/textures/DefaultWhiteTexture.dds", &m_pWhiteTexture));

	m_light.m_vLight   = D3DXVECTOR3(20.0f, 300.0f, 50.0f);
	m_light.m_ambient  = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	m_light.m_diffuse  = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	m_light.m_specular = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);

	D3DXVec3Normalize(&m_light.m_vLight, &m_light.m_vLight);

	BuildEffect();

	m_vLook		= D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_vRight	= D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	m_vUp		= D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	m_vPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_fScale = 0.3f;

	m_fRotAngleX = 0;
	m_fRotAngleY = 0;
	m_fRotAngleZ = 0;

	m_strModelName = strModelName;
	m_strModelFileName = ModelFileName;
	m_strTextureFileName = strTextureFileName;
	m_bIsBindable = false;
	m_strBindedToAnimatedModelName = "";
	m_strBindedToBoneName = "";
	D3DXCreateTextureFromFile(pApp->GetDevice(), strTextureFileName.c_str(), &m_pTexture);

	/*if( pMesh->m_bIsBindable && !pMesh->m_strBindedToAnimatedModelName.empty() && !pMesh->m_strBindedToBoneName.empty() ) {
		SkinnedModel* pSkinnedModel = static_cast<SkinnedModel*>(pApp->GetGameObjManager()->GetGameObjects().find(pMesh->m_strBindedToAnimatedModelName)->second);
		pSkinnedModel->BindWeaponToModel(pMesh->m_strModelName,pMesh->m_strBindedToBoneName);
	}*/

	m_eGameObjectType = EGameObjectType_Static;
	m_bIsPicked = false;
}

/////////////////////////////////////////////////////////////////////////

StaticModel::~StaticModel() {
	Destroy();
}

/////////////////////////////////////////////////////////////////////////

void StaticModel::LoadGameObject() {
	ID3DXMesh* pMesh			 = NULL;
	ID3DXBuffer* pMaterialBuffer = NULL;

	DWORD nMaterialsAmount = 0;

	CheckSuccess(D3DXLoadMeshFromX(m_strModelFileName.c_str(), D3DXMESH_SYSTEMMEM, pApp->GetDevice(), 0, &pMaterialBuffer, 0, &nMaterialsAmount, &pMesh));

	bool bHasNormals = HasNormals(pMesh);

	D3DVERTEXELEMENT9 elements[64];
	UINT nElementsAmount = 0;

	VertexPositionNormalTexture vertexDeclaration;
	pApp->GetPNTDecl()->GetDeclaration(elements, &nElementsAmount);

	ID3DXMesh* pTempMesh = 0;
	pMesh->CloneMesh(D3DXMESH_SYSTEMMEM, elements, pApp->GetDevice(), &pTempMesh);
	pMesh->Release();
	pMesh = pTempMesh;

	//generate normals if the models doesnt have
	if (!bHasNormals) {
		D3DXComputeNormals(pMesh, 0);
	}

	pMesh->CloneMesh(D3DXMESH_SYSTEMMEM, elements, pApp->GetDevice(), &(m_pMesh));
	pMesh->Release();

	//one mesh can be divided by different parts, called subsets and for every substet
	//there is different material and different texture associated with it
	//for example car can be divided by 3 parts - the car, front rims and back rims. 
	//These 3 subsets will have different material and different texture associated with them
	//this code reads from the information, stored in the .x file and saves it in two vectors: mMtrl and mTex
	if (pMaterialBuffer != 0 && nMaterialsAmount != 0) {
		D3DXMATERIAL* d3dxmtrls = (D3DXMATERIAL*)pMaterialBuffer->GetBufferPointer();

		for (DWORD i = 0; i < nMaterialsAmount; ++i) {
			Material material;
			material.m_ambient		  = d3dxmtrls[i].MatD3D.Diffuse;
			material.m_diffuse		  = d3dxmtrls[i].MatD3D.Diffuse;
			material.m_specular		  = d3dxmtrls[i].MatD3D.Specular;
			material.m_fSpecularPower = d3dxmtrls[i].MatD3D.Power;

			m_vMaterials.push_back(material);

			if (d3dxmtrls[i].pTextureFilename != 0) {
				IDirect3DTexture9* pTexture = 0;
				char strTexturePath[80];
				strcpy_s(strTexturePath,"../../Resources/textures/StaticModels/");
				strcat_s(strTexturePath,d3dxmtrls[i].pTextureFilename);
				CheckSuccess(D3DXCreateTextureFromFile(pApp->GetDevice(), strTexturePath, &pTexture));

				m_vTextures.push_back(pTexture);
			}
			else {
				m_vTextures.push_back(NULL);
			}
		}
	}
	ReleaseX(pMaterialBuffer);

	//create bounding box
	BuildBoundingBox();
}

/////////////////////////////////////////////////////////////////////////

void StaticModel::BuildBoundingBox() {
	VertexPositionNormalTexture* pVertexBuffer = 0;
	m_pMesh->LockVertexBuffer(0, (void**)&pVertexBuffer);

	D3DXComputeBoundingBox(&pVertexBuffer[0].m_pos, m_pMesh->GetNumVertices(), sizeof(VertexPositionNormalTexture), &(m_BoundingBox.GetMinPoint()), &(m_BoundingBox.GetMaxPoint()));
	
	m_pMesh->UnlockVertexBuffer();
	
    float width  = m_BoundingBox.GetMaxPoint().x - m_BoundingBox.GetMinPoint().x;
	float height = m_BoundingBox.GetMaxPoint().y - m_BoundingBox.GetMinPoint().y;
	float depth  = m_BoundingBox.GetMaxPoint().z - m_BoundingBox.GetMinPoint().z;

	pApp->GetLogStream() <<"Bounding box\n"<<width<<std::endl<<height<<std::endl<<depth<<std::endl;

	D3DXCreateBox(pApp->GetDevice(), width, height, depth, &m_pBoundingBoxMesh, 0);

	D3DXVECTOR3 center = m_BoundingBox.GetCenter();
	D3DXMatrixTranslation(&(m_BoundingBoxOffset), center.x, center.y, center.z);

	m_BoundingBoxMaterial.m_ambient = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	//below 0.4 does not work...
	m_BoundingBoxMaterial.m_diffuse = D3DXCOLOR(0.5f, 0.5f, 0.5f, 0.4f);
	m_BoundingBoxMaterial.m_specular = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	m_BoundingBoxMaterial.m_fSpecularPower = 8.0f;
}


/////////////////////////////////////////////////////////////////////////

void StaticModel::OnResetDevice() {
	m_pEffect->OnResetDevice();
}

/////////////////////////////////////////////////////////////////////////

void StaticModel::OnLostDevice() {
	//add check if it already went through here
	m_pEffect->OnLostDevice();
}

/////////////////////////////////////////////////////////////////////////

void StaticModel::OnUpdate(float fDeltaTime) {
}

/////////////////////////////////////////////////////////////////////////

void StaticModel::BuildEffect() {
	CheckSuccess(D3DXCreateEffectFromFile(pApp->GetDevice(), "../../Resources/shaders/StaticModelShader.fx", 0, 0, D3DXSHADER_DEBUG, 0, &m_pEffect, 0));

	m_hEffectTechnique	= m_pEffect->GetTechniqueByName("StaticModelTech");
	m_hWVPMatrix		= m_pEffect->GetParameterByName(0, "WVP");
	m_hMaterial			= m_pEffect->GetParameterByName(0, "mtrl");
	m_hLight			= m_pEffect->GetParameterByName(0, "light");
	m_hTexture			= m_pEffect->GetParameterByName(0, "text");
	m_hIsPicked			= m_pEffect->GetParameterByName(0, "picked");

	m_pEffect->SetValue(m_hLight, &m_light, sizeof(Light));
}

/////////////////////////////////////////////////////////////////////////

//renders not-binded model
void StaticModel::OnRender(const std::unique_ptr<Camera>& camera) {
	//if the model is not binded to skinned mesh's bone render it
	if (!m_bIsBindable) {
		m_pEffect->SetTechnique(m_hEffectTechnique);

		D3DXMATRIX T,T1;
		D3DXMATRIX S;
		D3DXMATRIX R;
		D3DXMATRIX R1,R2,R3;

		D3DXMatrixTranslation(&T,m_vPos.x,m_vPos.y,m_vPos.z);
		D3DXMatrixScaling(&S,m_fScale,m_fScale,m_fScale);
		D3DXMatrixRotationX(&R1,m_fRotAngleX);
		D3DXMatrixRotationY(&R2, m_fRotAngleY);
		D3DXMatrixRotationZ(&R3, m_fRotAngleZ);

		m_CombinedTransformationMatrix = S*R1*R2*R3*T;

		//alpha channel is used in the trees
		pApp->GetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, true);
		pApp->GetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		pApp->GetDevice()->SetRenderState(D3DRS_ALPHAREF, 100);

		D3DXMATRIX finalMatrix = m_CombinedTransformationMatrix * camera->GetViewProjMatrix();
		m_pEffect->SetMatrix(m_hWVPMatrix, &finalMatrix);

		if (pApp->GetGameObjManager()->ShouldHighlightPickedObjects()) {
			m_pEffect->SetBool(m_hIsPicked,m_bIsPicked);
		}

		UINT numPasses = 0;
		m_pEffect->Begin(&numPasses,0);
		for(UINT i =0;i<numPasses;++i) {
			m_pEffect->BeginPass(i);
				for(unsigned int j = 0; j < m_vMaterials.size(); ++j) {
					m_pEffect->SetValue(m_hMaterial, &(m_vMaterials[j]), sizeof(Material));
					if(m_vTextures[j] != NULL) {
						m_pEffect->SetTexture(m_hTexture, m_vTextures[j]);
					}
					else {
						m_pEffect->SetTexture(m_hTexture, m_pWhiteTexture);
					}
					m_pEffect->CommitChanges();
					m_pMesh->DrawSubset(j);					
				}

				if (pApp->GetGameObjManager()->ShouldRenderBoundingBoxes()) {
					RenderBoundingBox(camera);
				}
			m_pEffect->EndPass();
		}
		m_pEffect->End();
	}
	pApp->GetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE, false);
}

//TODO: do we need separate function for binded weapons? may be pass a parameter if it is binded or not?
/////////////////////////////////////////////////////////////////////////

//renders binded model
void StaticModel::RenderBindedWeapon(GameObject* pSkMesh, std::string bone, const std::unique_ptr<Camera>& camera) {
	//the animated model
	SkinnedModel* pSkinnedModel = static_cast<SkinnedModel*>(pSkMesh);

	//the binded weapon
	GameObject* pBindedObject = this;

	//the bone in the animated model's hierarchy
	Bone* pBone = static_cast<Bone*>(D3DXFrameFind(pSkinnedModel->GetRootFrame(), bone.c_str()));

	//testing variables
	/*static float angleX = 33.72;
	static float angleY = 9.129;
	static float angleZ = 10.6;
	static D3DXVECTOR3 pos = D3DXVECTOR3(57.8,38.8,-77.1);*/

	//matrices for the binded weapon
	D3DXMATRIX SS;
	D3DXMatrixScaling(&SS,pBindedObject->GetScale(),pBindedObject->GetScale(),pBindedObject->GetScale());

	D3DXMATRIX R1SX;
	D3DXMatrixRotationX(&R1SX,pBindedObject->GetRotationAngleByX());
	D3DXMATRIX R1SY;
	D3DXMatrixRotationY(&R1SY,pBindedObject->GetRotationAngleByY());
	D3DXMATRIX R1SZ;
	D3DXMatrixRotationZ(&R1SZ,pBindedObject->GetRotationAngleByZ());

	D3DXMATRIX TS;
	D3DXMatrixTranslation(&TS,pBindedObject->GetPosition().x,pBindedObject->GetPosition().y,pBindedObject->GetPosition().z);

	//combined matrix
	D3DXMATRIX BindedObjectCombinedMatrix = SS*(R1SY*R1SX*R1SZ)*TS;

	//this part can be used for manual control over the binded weapon through keyboard.
	//outputs the results in the log files
	/*if(Dinput->keyDown(DIK_C))
	{
		pos.x+=0.1;
	}
	if(Dinput->keyDown(DIK_V))
	{
		pos.x-=0.1;
	}
	if(Dinput->keyDown(DIK_B))
	{
		pos.y+=0.1;
	}
	if(Dinput->keyDown(DIK_N))
	{
		pos.y-=0.1;
	}
	if(Dinput->keyDown(DIK_M))
	{
		pos.z+=0.1;
	}
	if(Dinput->keyDown(DIK_K))
	{
		pos.z-=0.1;
	}

	//static float angleY = -9;
	//static float angleX = -1.5;
						
					
	if(Dinput->keyDown(DIK_Y))
	{
		angleY+=0.01;
	}
	if(Dinput->keyDown(DIK_U))
	{
		angleY-=0.01;
	}
	if(Dinput->keyDown(DIK_O))
	{
		angleZ+=0.01;
	}
	if(Dinput->keyDown(DIK_P))
	{
		angleZ-=0.01;
	}
	if(Dinput->keyDown(DIK_R))
	{
		angleX+=0.01;
	}
	if(Dinput->keyDown(DIK_T))
	{
		angleX-=0.01;
	}
						
	pApp->GetLogStream()<<"rot_angleX:"<<angleX<<endl;
	pApp->GetLogStream()<<"rot_angleY:"<<angleY<<endl;
	pApp->GetLogStream()<<"rot_angleZ:"<<angleZ<<endl;
	pApp->GetLogStream()<<"posx:"<<pos.x<<endl;
	pApp->GetLogStream()<<"posy:"<<pos.y<<endl;
	pApp->GetLogStream()<<"posz:"<<pos.z<<endl;*/
						

	//matrices for the animated model
	D3DXMATRIX TA;
	D3DXMatrixTranslation(&TA,pSkinnedModel->GetPosition().x,pSkinnedModel->GetPosition().y,pSkinnedModel->GetPosition().z);
	D3DXMATRIX SA;
	D3DXMatrixScaling(&SA,pSkinnedModel->GetScale(),pSkinnedModel->GetScale(),pSkinnedModel->GetScale());
	D3DXMATRIX R1A;
	D3DXMatrixRotationY(&R1A,pSkinnedModel->GetRotationAngleByY());
	D3DXMATRIX AnimatedModelCombinedMatrix = SA*R1A*(TA);

	//combined matrix for the bone
	D3DXMATRIX BoneCombinedMatrix = pBone->TransformationMatrix*pBone->m_toRootMatrix;

	m_pEffect->SetTechnique(m_hEffectTechnique);

	//the full matrix for the binded weapon. 
	//Its combination from bone matrices, animated model matrices and the binded weapon matrices
	D3DXMATRIX FullCombinedMatrix = BindedObjectCombinedMatrix * BoneCombinedMatrix * AnimatedModelCombinedMatrix;

	D3DXMATRIX finalMatrix = FullCombinedMatrix * camera->GetViewProjMatrix();
	m_pEffect->SetMatrix(m_hWVPMatrix, &finalMatrix);
						
	UINT numPasses = 0;
	m_pEffect->Begin(&numPasses,0);
	for(UINT i =0;i<numPasses;++i) {
		m_pEffect->BeginPass(i);

			for(unsigned int j = 0; j < pBindedObject->GetMaterials().size(); ++j) {
				m_pEffect->SetValue(m_hMaterial, &(pBindedObject->GetMaterials()[j]), sizeof(Material));
				if(pBindedObject->GetTextures()[j] != 0) {
					m_pEffect->SetTexture(m_hTexture, pBindedObject->GetTextures()[j]);
				}
				else {
					m_pEffect->SetTexture(m_hTexture, m_pWhiteTexture);
				}
													
				m_pEffect->CommitChanges();
				pBindedObject->GetMesh()->DrawSubset(j);
			}

			pApp->GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			pApp->GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pApp->GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			//bounding box render for binded weapon
			D3DXMATRIX finalMatrix = pBindedObject->GetBBOffsetMatrix() * FullCombinedMatrix * camera->GetViewProjMatrix();
			m_pEffect->SetMatrix(m_hWVPMatrix, &finalMatrix);
			Material bbMaterial = pBindedObject->GetBBMaterial();
			m_pEffect->SetValue(m_hMaterial, &bbMaterial, sizeof(Material));
			m_pEffect->SetTexture(m_hTexture, m_pWhiteTexture);
			m_pEffect->CommitChanges();
			//pBindedObject->m_pBoundingBoxMesh->DrawSubset(0);
			pApp->GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

		m_pEffect->EndPass();
	}
	m_pEffect->End();		
}

/////////////////////////////////////////////////////////////////////////

void StaticModel::RenderBoundingBox(const std::unique_ptr<Camera>& camera) {
	pApp->GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pApp->GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pApp->GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	D3DXMATRIX T,T1;
	D3DXMATRIX S;
	D3DXMATRIX R;
	D3DXMATRIX R1,R2,R3;

	D3DXMatrixTranslation(&T,m_vPos.x,m_vPos.y,m_vPos.z);
	D3DXMatrixScaling(&S,m_fScale,m_fScale,m_fScale);
	D3DXMatrixRotationX(&R1,m_fRotAngleX);
	D3DXMatrixRotationY(&R2, m_fRotAngleY);
	D3DXMatrixRotationZ(&R3, m_fRotAngleZ);

	D3DXMATRIX meshCombined = m_BoundingBoxOffset*S*R1*R2*R3*T;

	m_BoundingBox.m_transformationMatrix = meshCombined;
	D3DXMATRIX finalMatrix = meshCombined * camera->GetViewProjMatrix();
	m_pEffect->SetMatrix(m_hWVPMatrix, &finalMatrix);

	m_pEffect->SetValue(m_hMaterial, &m_BoundingBoxMaterial, sizeof(Material));
	m_pEffect->SetTexture(m_hTexture, m_pWhiteTexture);
	m_pEffect->CommitChanges();

	m_pBoundingBoxMesh->DrawSubset(0);

	pApp->GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
}

/////////////////////////////////////////////////////////////////////////

float StaticModel::GetDistanceToPickedObject(const std::unique_ptr<Camera>& camera) {
	D3DXVECTOR3 vOrigin(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vDir(0.0f, 0.0f, 0.0f);

	camera->GetWorldPickingRay(vOrigin, vDir);
	bool bIsPicked = false;
	AABB box = m_BoundingBox.TransformByMatrix(m_CombinedTransformationMatrix);
	bIsPicked = D3DXBoxBoundProbe(&box.GetMinPoint(), &box.GetMaxPoint(), &vOrigin, &vDir);

	if (bIsPicked) {
		//by finding the inverse of mesh CombinedMatrix we transform from World space to the mesh local space
		//this is needed, because we have to transform the picking vectors to the mesh local space
		//which is required by the D3DXIntersect function
		D3DXMATRIX InverseWorldMatrix;
		D3DXMatrixInverse(&InverseWorldMatrix, 0, &m_CombinedTransformationMatrix);

		//transform the Ray using the inverse matrix
		D3DXVec3TransformCoord(&vOrigin, &vOrigin, &InverseWorldMatrix);
		D3DXVec3TransformNormal(&vDir, &vDir, &InverseWorldMatrix);

		BOOL hit = false;
		DWORD faceIndex = -1;
		float u = 0.0f;
		float v = 0.0f;
		float dist = FLT_MAX;

		ID3DXBuffer* allhits = nullptr;
		DWORD numHits = 0;
		D3DXIntersect(m_pMesh, &vOrigin, &vDir, &hit, &faceIndex, &u, &v, &dist, &allhits, &numHits);
		ReleaseX(allhits);

		return dist;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////

bool StaticModel::SpawnClone() {
	std::shared_ptr<StaticModel> pMesh = std::make_shared<StaticModel>();
	pMesh->SetPosition(GetPosition());
	pMesh->SetScale(GetScale());
	pMesh->SetRotationAngleByX(GetRotationAngleByX());
	pMesh->SetRotationAngleByY(GetRotationAngleByY());
	pMesh->SetRotationAngleByZ(GetRotationAngleByZ());
	pMesh->SetIsBindable(IsBindable());
	pMesh->SetBindedToAnimatedModelName(GetBindedToAnimatedModelName());
	pMesh->SetBindedToBoneName(GetBindedToBoneName());
	pMesh->SetModelFilename(GetModelFileName());
	pMesh->SetPicked(false);
	pMesh->LoadGameObject();
	pMesh->SetObjectType(EGameObjectType_Static);

	pApp->GetGameObjManager()->AddGameObject(pMesh);
	return true;
}

/////////////////////////////////////////////////////////////////////////

void StaticModel::Destroy() {
	m_pWhiteTexture->Release();
	m_pEffect->Release();
}

/////////////////////////////////////////////////////////////////////////

void StaticModel::UpdateGameObjectHeightOnTerrain(const std::unique_ptr<Terrain>& terrain) {
	//binded models got their own height, based on the bone that they are attached to, other models are just put on the terrain
	if (!m_bIsBindable && pApp->GetGameObjManager()->AreObjectsGrounded()) {
		m_vPos.y = terrain->GetHeight(m_vPos.x, m_vPos.z);
	}
}

/////////////////////////////////////////////////////////////////////////