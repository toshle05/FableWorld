#include <stdafx.h>
#include"GameObject.h"

GameObject::GameObject()
{
	m_id = -1;
}

GameObject::~GameObject()
{
	m_pMesh->Release();
	for (auto& texture : m_vTextures)
	{
		texture->Release();
	}
	m_pBoundingBoxMesh->Release();
}

void GameObject::LoadGameObject()
{
	
}

void GameObject::OnUpdate(float dt)
{
	
}

void GameObject::OnRender()
{
	
}

//invoked when alt+tab is pressed for instance
void GameObject::OnLostDevice()
{
	
}

//when we restore from alt+tab
void GameObject::OnResetDevice()
{
	
}

void GameObject::RenderBindedWeapon(GameObject* pSkMesh, std::string bone)
{
	
}

//checks if object is picked
float GameObject::GetDistanceToPickedObject()
{
	return -1;
}

std::string GameObject::GetModelFileName() const
{
	return m_strModelFileName;
}

void GameObject::SetModelFilename(const std::string& strModelFileName)
{
	m_strModelFileName = strModelFileName;
}

std::string GameObject::GetName() const
{
	return m_strModelName;
}

void GameObject::SetName(const std::string& strName)
{
	m_strModelName = strName;
}

int GameObject::GetId() const
{
	return m_id;
}

void GameObject::SetId(int id)
{
	m_id = id;
}

D3DXVECTOR3 GameObject::GetPosition() const
{
	return m_vPos;
}

void GameObject::SetPosition(const D3DXVECTOR3& pos)
{
	m_vPos = pos;
}

D3DXVECTOR3& GameObject::GetLookVector()
{
	return m_vLook;
}

D3DXVECTOR3& GameObject::GetRightVector()
{
	return m_vRight;
}

D3DXVECTOR3& GameObject::GetUpVector()
{
	return m_vUp;
}

float GameObject::GetScale() const
{
	return m_fScale;
}

void GameObject::SetScale(float scale)
{
	m_fScale = scale;
}

void GameObject::ModifyScale(float delta)
{
	m_fScale += delta;
}

ID3DXMesh* GameObject::GetMesh() const
{
	return m_pMesh;
}

void GameObject::SetMesh(ID3DXMesh* mesh)
{
	m_pMesh = mesh;
}

std::vector<Material>& GameObject::GetMaterials()
{
	return m_vMaterials;
}

void GameObject::SetMaterials(std::vector<Material> materials)
{
	m_vMaterials = materials;
}

std::vector<IDirect3DTexture9*>& GameObject::GetTextures()
{
	return m_vTextures;
}

void GameObject::SetTextures(std::vector<IDirect3DTexture9*> textures)
{
	m_vTextures = textures;
}

IDirect3DTexture9* GameObject::GetTexture() const
{
	return m_pTexture;
}

void GameObject::SetTexture(IDirect3DTexture9* texture)
{
	m_pTexture = texture;
}

std::string GameObject::GetTextureFilename() const
{
	return m_strTextureFileName;
}

void GameObject::SetTextureFilename(const std::string& filename)
{
	m_strTextureFileName = filename;
}

int GameObject::IsBindable() const
{
	return m_bIsBindable;
}

void GameObject::SetIsBindable(int bindable)
{
	m_bIsBindable = bindable;
}

std::string GameObject::GetBindedToAnimatedModelName() const
{
	return m_strBindedToAnimatedModelName;
}

void GameObject::SetBindedToAnimatedModelName(const std::string& bindedToAnimatedModelName)
{
	m_strBindedToAnimatedModelName = bindedToAnimatedModelName;
}

std::string GameObject::GetBindedToBoneName() const
{
	return m_strBindedToBoneName;
}

void GameObject::SetBindedToBoneName(const std::string& bindedToBoneName)
{
	m_strBindedToBoneName = bindedToBoneName;
}

void GameObject::TransformByMatrix(D3DXMATRIX matrix)
{
	D3DXVec3TransformCoord(&m_vLook,	&m_vLook,	&matrix);
	D3DXVec3TransformCoord(&m_vRight,	&m_vRight,	&matrix);
	D3DXVec3TransformCoord(&m_vUp,		&m_vUp,		&matrix);
}

float GameObject::GetRotationAngleByX() const
{
	return m_fRotAngleX;
}

void GameObject::SetRotationAngleByX(float rotationAngleByX)
{
	m_fRotAngleX = rotationAngleByX;
}

void GameObject::ModifyRotationAngleByX(float delta)
{
	m_fRotAngleX += delta;
}

float GameObject::GetRotationAngleByY() const
{
	return m_fRotAngleY;
}

void GameObject::SetRotationAngleByY(float rotationAngleByY)
{
	m_fRotAngleY = rotationAngleByY;
}


void GameObject::ModifyRotationAngleByY(float delta)
{
	m_fRotAngleY += delta;
}

float GameObject::GetRotationAngleByZ() const
{
	return m_fRotAngleZ;
}

void GameObject::SetRotationAngleByZ(float rotationAngleByZ)
{
	m_fRotAngleZ = rotationAngleByZ;
}

void GameObject::ModifyRotationAngleByZ(float delta)
{
	m_fRotAngleZ += delta;
}

bool GameObject::IsPicked() const
{
	return m_bIsPicked;
}

void GameObject::SetPicked(bool picked)
{
	m_bIsPicked = picked;
}

ID3DXMesh* GameObject::GetBBMesh() const
{
	return m_pBoundingBoxMesh;
}

void GameObject::SetBBMesh(ID3DXMesh* BBMesh)
{
	m_pBoundingBoxMesh = BBMesh;
}

Material GameObject::GetBBMaterial() const
{
	return m_BoundingBoxMaterial;
}

void GameObject::SetBBMaterial(Material material)
{
	m_BoundingBoxMaterial = material;
}

AABB GameObject::GetBB() const
{
	return m_BoundingBox;
}

void GameObject::SetBB(AABB boundingBox)
{
	m_BoundingBox = boundingBox;
}

D3DXMATRIX GameObject::GetBBOffsetMatrix() const
{
	return m_BoundingBoxOffset;
}

void GameObject::SetBBOffsetMatrix(D3DXMATRIX offsetMatrix)
{
	m_BoundingBoxOffset = offsetMatrix;
}

EGameObjectType GameObject::GetObjectType() const
{
	return m_eGameObjectType;
}

void GameObject::SetObjectType(EGameObjectType type)
{
	m_eGameObjectType = type;
}

D3DXMATRIX GameObject::GetCombinedTransfMatrix() const
{
	return m_CombinedTransformationMatrix;
}

void GameObject::SetCombinedTransfMatrix(D3DXMATRIX mat)
{
	m_CombinedTransformationMatrix = mat;
}