#pragma once
#include<stdafx.h>

//////////////////////////////////////////////////////////////////////////////

class GameObject;
class IBaseMenuObject;
class GameObjectManager;
class DirectInput;
class TextManager;

//////////////////////////////////////////////////////////////////////////////
      
class Application {
public:
	Application(HINSTANCE hInstance, std::string strWindowTitle, D3DDEVTYPE eDeviceType, DWORD requestedVP);
	~Application();
	HINSTANCE				GetAppInstance();
	HWND					GetMainWindow();
	D3DPRESENT_PARAMETERS	GetPresentParameters();
	int						MainLoop();
	bool					IsDeviceLost();
	bool					IsPaused() const;
	void					SetPaused(bool bPaused);
	void					SwitchToFullscreen(bool bSwitch);
	void					SetCurrentScene(IBaseScene* pScene);
	IBaseScene*				GetCurrentScene();
	void					AddScene(std::string strSceneName, IBaseScene* pScene);
	IBaseScene*				GetScene(std::string strSceneName);
	bool					IsShaderVersionSupported();

	void					AddUIObject(IBaseMenuObject* pUIObject);
	std::vector<IBaseMenuObject*> m_vUIObjects;
	//holds the selected textbox, this must not be here,
	//MenuManager must be made instead and it has to be moved there.
	std::string					m_strSelectedTextbox;
	IBaseMenuObject*		FindMenuObject(std::string strObjectId);

	void					InitManagers();
	auto					GetGameObjManager() -> const std::unique_ptr<GameObjectManager>&;
	auto					GetDinput() -> const std::unique_ptr<DirectInput>&;
	auto					GetTextManager() -> const std::unique_ptr<TextManager>&;
	std::ofstream&			GetLogStream();
	IDirect3DDevice9*		GetDevice();

public:
	IDirect3DVertexDeclaration9* GetPNTDecl();
	IDirect3DVertexDeclaration9* GetPCDecl();
	IDirect3DVertexDeclaration9* GetParticleDecl();
	IDirect3DVertexDeclaration9* GetPositionNormalDisplacementDecl();

private:
	void					InitMainWindow();
	void					InitDirect3D();
	void					InitVertexDeclarations();

private:
	//the std::string in the title bar if we are in windowed mode
	std::string					 m_strWindowTitle;
	//handle to the current application instance.
	HINSTANCE				m_hAppInstance;
	//handle to the current window
	HWND					m_hMainWindow;
	//the type of the device we want to create - HAL, SW, REF
	D3DDEVTYPE				m_eDeviceType;
	//the type of vertex processing - software vertex processing or hardware vertex processing
	DWORD					m_vertexProcessingType;
	IDirect3D9*				m_pD3DObject;
	//present parameters - used for describing the directx device we want to create
	D3DPRESENT_PARAMETERS	m_presentParameters;
	bool					m_bIsAppPaused;
	//map containing all the scenes in the game, currently Menu scene, Game scene and MenuInGame Scene
	std::map<std::string,IBaseScene*> m_mapScenesContainer;
	//pointer to the current scene
	IBaseScene*				m_pCurrentScene;
	IDirect3DVertexDeclaration9* m_pVertexPNTDecl;
	IDirect3DVertexDeclaration9* m_pVertexPCDecl;
	IDirect3DVertexDeclaration9* m_pVertexParticleDecl;
	IDirect3DVertexDeclaration9* m_pVertexPositionNormalDisplacementDecl;
	std::unique_ptr<GameObjectManager> m_pGameObjManager;
	std::unique_ptr<DirectInput> m_pDinput;
	std::unique_ptr<TextManager> m_pTextManager;
	std::ofstream     m_LogStream;
	IDirect3DDevice9* m_pDxDevice;
};

//////////////////////////////////////////////////////////////////////////////

extern Application*		 pApp;

//////////////////////////////////////////////////////////////////////////////

struct VertexPosition {
	D3DXVECTOR3 m_pos;
};

//////////////////////////////////////////////////////////////////////////////

struct VertexPositionColor {
	D3DXVECTOR3 m_pos;
	D3DXCOLOR   m_color;
};

//////////////////////////////////////////////////////////////////////////////

struct VertexPositionNormal {
	D3DXVECTOR3 m_pos;
	D3DXVECTOR3 m_vNormal;
};

//////////////////////////////////////////////////////////////////////////////

//used for water
struct VertexPositionNormalDisplacement {
	D3DXVECTOR3 m_pos;
	D3DXVECTOR2 m_normalMapCoord;       // [a, b]
	D3DXVECTOR2 m_displacementMapCoord; // [0, 1]
};

//////////////////////////////////////////////////////////////////////////////

struct VertexPositionNormalTexture {
	D3DXVECTOR3 m_pos;
	D3DXVECTOR3 m_normal;
	D3DXVECTOR2 m_textureCoord;
};

//////////////////////////////////////////////////////////////////////////////
