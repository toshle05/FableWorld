#include <stdafx.h>
#include "Game.h"
#include "QuestManager.h"
#include "Navmesh.h"

/////////////////////////////////////////////////////////////////////////

Game::Game() {
	if (!pApp->IsShaderVersionSupported()) {
		MessageBox(0, "Shader version is not supported", 0, 0);
		PostQuitMessage(0); 
	}

	InitUI();
	InitCamera();
	m_pSky = std::make_unique<Sky>("../../Resources/textures/Sky/grassenvmap1024.dds", 10000.0f);
	InitTerrain();
	pApp->GetTextManager()->CreateFontFor3DText();
	InitLua();

	m_pDialogueManager = std::make_unique<DialogueManager>();
	m_pDialogueManager->LoadDialogues("../../Resources/dialogues/dialogue.xml");

	InitGameObjects();
	InitDebugGraphicsShader();
	m_pGunEffect = std::make_unique<GunEffect>("../../Resources/shaders/Effects/GunShader.fx","GunEffectTech","../../Resources/textures/Effects/bolt.dds",100, D3DXVECTOR4(0, -9.8f, 0.0f,0.0f));
	
	m_isAIRunningToTarget = false;
	m_AIIntersectPoint = D3DXVECTOR3(0, 0, 0);

	InitNavmesh();
	InitWater();
}

/////////////////////////////////////////////////////////////////////////

//initializes the shader for debug graphics
void Game::InitDebugGraphicsShader() {
	D3DXCreateEffectFromFile(pApp->GetDevice(), "../../Resources/shaders/DebugGraphicsShader.fx", 0, 0, D3DXSHADER_DEBUG, 0, &m_pDebugGraphicsEffect, 0);
	m_hDebugGraphicsTechnique  = m_pDebugGraphicsEffect->GetTechniqueByName("DebugGraphics3DTech");
	m_hDebugGraphicsWVPMatrix  = m_pDebugGraphicsEffect->GetParameterByName(0, "WVP");
}

/////////////////////////////////////////////////////////////////////////

void Game::InitCamera() {
	const float fWidth  = static_cast<float>(pApp->GetPresentParameters().BackBufferWidth);
	const float fHeight = static_cast<float>(pApp->GetPresentParameters().BackBufferHeight);

	m_pCamera = std::make_unique<Camera>(D3DX_PI * 0.25f, fWidth / fHeight, 1.0f, 20000, true);
	m_pCamera->SetCameraMode(ECameraMode::MoveWithPressedMouse);
	m_pCamera->SetPosition(D3DXVECTOR3(-1058, 1238, 674));
	m_pCamera->SetSpeed(500);
	m_pCamera->RotateUp(0.8);
	m_pCamera->RotateRight(0.6);
}

/////////////////////////////////////////////////////////////////////////

void Game::InitTerrain() {
	m_pTerrain = std::make_unique<Terrain>("../../Resources/heightmaps/heightmap_new.raw", 0.5f, 513, 513, 4, 4, D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	//m_pTerrain = std::make_unique<Terrain>("../../Resources/heightmaps/coastMountain1025.raw", 1.0f, 1025, 1025, 10.0f, 10.0f, D3DXVECTOR3(0.0f, 0.0f, 0.0f));

	//the direction to the sun
	D3DXVECTOR3 lightVector(20.0f, 300.0f, 50.0f);
	D3DXVec3Normalize(&lightVector, &lightVector);
	m_pTerrain->SetLightVector(lightVector);
}

/////////////////////////////////////////////////////////////////////////

void Game::InitLua() {
	//init lua
	m_pLuaState = lua_open();
	//open lua libs
	luaL_openlibs(m_pLuaState);

	//with lua_register we bind the functions in the cpp file with the invoked functions in the script file
	//here addStaticModel is function in the script.When it is invoked there it actually invokes l_addStaticModel(lua_State* L)
	lua_register(m_pLuaState, "addStaticModel", l_addStaticModel);
	lua_register(m_pLuaState, "addAnimatedModel", l_addAnimatedModel);
	lua_register(m_pLuaState, "addQuest", l_addQuest);
	//loads the models, sounds and quests from the scripts
	luaL_dofile(m_pLuaState, "../../Resources/levels/levelInGame_new.lua");
	luaL_dofile(m_pLuaState, "../../Resources/scripts/quests.lua");
}

/////////////////////////////////////////////////////////////////////////

void Game::InitGameObjects() {
	auto& gameObjects = pApp->GetGameObjManager()->GetSkinnedModels();
	for (auto& gameObject : gameObjects) {
		//create mesh for 3d text above the models
		pApp->GetTextManager()->CreateMeshFor3DText(gameObject);

		for (auto& dialogue : m_pDialogueManager->GetDialogues()) {
			if (!gameObject->GetName().compare(dialogue->m_strModel)) {
				gameObject->SetHasDialogue(true);
				pApp->GetTextManager()->CreateMeshFor3DTextQuest(gameObject);
			}
		}

		if (!gameObject->GetActorType().compare("mainHero")) {
			if (m_pMainHero != nullptr) {
				MessageBox(0, "Main hero already set. Fix the duplicate in the lua file", 0, 0);
				exit(0);
			}
			m_pMainHero = gameObject;
		}
	}

	pApp->GetGameObjManager()->SetPickedObject(m_pMainHero);

	//auto objCho = pApp->GetGameObjManager()->GetObjectByName("cho");
	//objCho->SpawnClone();//~7.5MB per one cho
	//objCho->SpawnClone();
	//objCho->SpawnClone();
	//objCho->SpawnClone();
	//objCho->SpawnClone();
	//objCho->SpawnClone();
}

/////////////////////////////////////////////////////////////////////////

void Game::InitWater() {
	D3DXMATRIX waterWorld;
	D3DXMatrixTranslation(&waterWorld, 0.0f, -400.0f, 0.0f);

	Material waterMtrl;
	waterMtrl.m_ambient = D3DXCOLOR(0.4f, 0.4f, 0.7f, 0.0f);
	waterMtrl.m_diffuse = D3DXCOLOR(0.4f, 0.4f, 0.7f, 1.0f);
	waterMtrl.m_specular = 1.0f * WHITE;
	waterMtrl.m_fSpecularPower = 128.0f;

	Light light;
	light.m_vLight = D3DXVECTOR3(0.0f, -1.0f, -3.0f);
	D3DXVec3Normalize(&light.m_vLight, &light.m_vLight);
	light.m_ambient  = D3DXCOLOR(0.5f, 0.5f, 0.6f, 1.0f);
	light.m_diffuse  = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.m_specular = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);

	Water::InitInfo waterInitInfo;
	waterInitInfo.dirLight = light;
	waterInitInfo.mtrl = waterMtrl;
	waterInitInfo.vertRows = 128;
	waterInitInfo.vertCols = 128;
	waterInitInfo.dx = 200.f;
	waterInitInfo.dz = 200.f;
	waterInitInfo.waveMapFilename0	= "../../Resources/textures/Water/wave0.dds";
	waterInitInfo.waveMapFilename1	= "../../Resources/textures/Water/wave1.dds";
	waterInitInfo.dmapFilename0		= "../../Resources/textures/Water/waterdmap0.dds";
	waterInitInfo.dmapFilename1		= "../../Resources/textures/Water/waterdmap1.dds";
	waterInitInfo.waveNMapVelocity0 = D3DXVECTOR2(0.05f, 0.07f);
	waterInitInfo.waveNMapVelocity1 = D3DXVECTOR2(-0.01f, 0.13f);
	waterInitInfo.waveDMapVelocity0 = D3DXVECTOR2(0.001f, 0.001f);
	waterInitInfo.waveDMapVelocity1 = D3DXVECTOR2(0.001f, 0.001f);
	waterInitInfo.scaleHeights		= D3DXVECTOR2(302.7f, 302.1f);
	waterInitInfo.texScale			= 8.0f;
	waterInitInfo.toWorld			= waterWorld;

	m_pWater = std::make_unique<Water>(waterInitInfo);
}

/////////////////////////////////////////////////////////////////////////

void Game::InitNavmesh() {
	m_pNavmesh = std::make_unique<Navmesh>();
	m_pNavmesh->resetCommonSettings();
	m_pNavmesh->loadGeometry();
	bool success = m_pNavmesh->handleBuild();
	if (!success) {
		printf("failed to build navmesh \n");
	}

	m_currentPathfindingEndIndex = 0;
}

/////////////////////////////////////////////////////////////////////////

void Game::InitUI() {
	//sprite for the interface in the game
	D3DXCreateSprite(pApp->GetDevice(), &m_pInterfaceSprite);
	//textures for the interface
	CheckSuccess(D3DXCreateTextureFromFile(pApp->GetDevice(), "../../Resources/textures/GUI/healthbar.dds", &m_pHealthBarTexture));
	CheckSuccess(D3DXCreateTextureFromFile(pApp->GetDevice(), "../../Resources/textures/GUI/healthbar_filled.dds", &m_pHealthBarFilledTexture));
	CheckSuccess(D3DXCreateTextureFromFile(pApp->GetDevice(), "../../Resources/textures/GUI/healthbar_filled_enemy.dds", &m_phealthBarFilledEnemyTexture));
	m_rHealthBarRectangle.left = 0;
	m_rHealthBarRectangle.top = 0;
	m_rHealthBarRectangle.right = 270;
	m_rHealthBarRectangle.bottom = 17;

	m_rEnemyHealthBarRectangle.left = 0;
	m_rEnemyHealthBarRectangle.top = 0;
	m_rEnemyHealthBarRectangle.right = 270;
	m_rEnemyHealthBarRectangle.bottom = 17;

	m_vHealthBarPosition = D3DXVECTOR3(100.0, 5.0, 0.0);
	m_vEnemyHealthBarPosition = D3DXVECTOR3(370, 5.0, 0.0);

	// hides the enemy health bar before we attack enemy.
	m_bIsEnemyHealthBarVisible = false;

	auto height = pApp->GetPresentParameters().BackBufferHeight;
	D3DXVECTOR2 spellPosition(100, height - 80);

	m_pHealSpell = std::make_unique<Button>(spellPosition, 64, 64, "", "heal1.dds", "heal1.dds");
}

/////////////////////////////////////////////////////////////////////////

void Game::OnUpdate(float dt) {
	//poll starts to listen if any key on the keyboard is pressed
	pApp->GetDinput()->Poll();
	if (pApp->GetDinput()->IsKeyDown(DIK_F)) {
		pApp->SwitchToFullscreen(true);
	}
	//if escape is pressed in game it switches to another scene
	if (pApp->GetDinput()->IsKeyDown(DIK_ESCAPE)) {
		IBaseScene* pMenuInGameScene = pApp->GetScene("menuInGame");
		pApp->SetCurrentScene(pMenuInGameScene);
	}

	//update all the game objects
	for (auto& gameObject : pApp->GetGameObjManager()->GetGameObjects()) {
		gameObject->OnUpdate(dt);
		gameObject->UpdateGameObjectHeightOnTerrain(m_pTerrain);
	}

	pApp->GetTextManager()->OnUpdate(dt);
	m_pCamera->OnUpdate(dt);
	
	m_pDialogueManager->OnUpdate();

	//checking if there is active quest
	for (auto& quest : GetQuestManager()->GetQuests()) {
		auto reqObject = pApp->GetGameObjManager()->GetSkinnedModelByName(quest->m_strRequiredObject);

		//if mainHero is close to the required from the quest object and the required object is dead the quest is completed.
		if (IsObjectNear(m_pMainHero->GetPosition(), reqObject->GetPosition()) && reqObject->IsDead()) {
			quest->m_bIsCompleted = true;
		}
	}

	//binding the camera to mainHero in the game and moving it. mainHero is set in the scripts in init.lua
	if (!m_pMainHero->IsDead() && !m_pCamera->IsCameraFree()) {
		MoveActor(m_pMainHero, dt);
	}

	//updating the models's titles positions
	for (auto& gameObject : pApp->GetGameObjManager()->GetSkinnedModels()) {
		D3DXVECTOR3 titleRightVector = gameObject->GetTitleRightVector();
		D3DXVECTOR3 cameraLookVector = m_pCamera->GetLookVector();
		float angle = D3DXVec3Dot(&titleRightVector,&cameraLookVector);
		gameObject->ModifyTitleRotationAnglyByY(angle);
		gameObject->ModifyTitleForQuestRotationAnglyByY(angle);

		D3DXMATRIX R;
		D3DXMatrixRotationY(&R, angle);

		gameObject->TransformTitleByMatrix(R);
	}

	//for (auto it : pApp->GetGameObjManager()->GetGameObjects()) {
	//	DrawLine(pMainHero->GetPosition(), it->GetPosition());
	//}

	UpdateAI(dt);

	if (pApp->GetDinput()->IsMouseButtonUp(0) && m_pHealSpell->IsClicked()) {
		m_rHealthBarRectangle.right += 20;
	}

	m_pHealSpell->OnUpdate();

	m_pGunEffect->OnUpdate(dt);

	static float delay = 0.0f;
	if (pApp->GetDinput()->IsKeyDown(DIK_SPACE) && delay <= 0.0f) {
		auto pickedObj = pApp->GetGameObjManager()->GetPickedObject();
		if (pickedObj) {
			delay = 0.1f;
			m_pGunEffect->AddParticle(pickedObj);
		}
	}

	auto pickedObj = pApp->GetGameObjManager()->GetPickedObject();

	if (pickedObj) {
		if (pApp->GetDinput()->IsMouseButtonDown(1)) {
			m_AIIntersectPoint = D3DXVECTOR3(0, 0, 0);
			D3DXVECTOR3 vOrigin(0.0f, 0.0f, 0.0f);
			D3DXVECTOR3 vDir(0.0f, 0.0f, 0.0f);

			m_pCamera->GetWorldPickingRay(vOrigin, vDir);

			D3DXPLANE plane;
			D3DXVECTOR3 vPoint(0.0f, 0.0f, 0.0f);
			D3DXVECTOR3 vNormal(0.0f, 1.0f, 0.0f);
			D3DXPlaneFromPointNormal(&plane, &vPoint, &vNormal);

			//I though that only origin and direction is needed and since
			//line is thought to be endless I am not going to need end point.
			//looks like we need it. I should really check the API next time, before assuming things.
			D3DXVECTOR3 lineEndPoint = vOrigin + INT_MAX * vDir;
			D3DXPlaneIntersectLine(&m_AIIntersectPoint, &plane, &vOrigin, &lineEndPoint);

			m_pNavmesh->FindPath(pickedObj->GetPosition(), m_AIIntersectPoint);

			m_currentPath.clear();
			m_currentPath = m_pNavmesh->GetCalculatedPath();

			m_currentPathfindingEndIndex = 0;
			m_isAIRunningToTarget = true;
		}
	}

	if (m_isAIRunningToTarget && pickedObj) {
		const int pathAdvanceSpeedModifier = 3;
		m_currentPathfindingEndIndex+= pathAdvanceSpeedModifier;
		auto pSkinnedModel = std::static_pointer_cast<SkinnedModel>(pickedObj);
		if (m_currentPathfindingEndIndex < m_currentPath.size()) {
			auto pos = m_currentPath[m_currentPathfindingEndIndex];

			pSkinnedModel->SetAnimationSpeed(2.8);
			pSkinnedModel->PlayAnimation("run");

			pSkinnedModel->AlignToDirection(pos);

			pSkinnedModel->SetPosition(pos);
		}
		else {
			pSkinnedModel->SetAnimationSpeed(1);
			pSkinnedModel->PlayAnimation("idle");

			m_isAIRunningToTarget = false;
		}
	}

	delay -= dt;

	m_pWater->OnUpdate(dt);
}

/////////////////////////////////////////////////////////////////////////

void Game::UpdateAI(float dt) {
	auto pEnemy = pApp->GetGameObjManager()->GetSkinnedModelByName(m_pMainHero->GetAttackerName());

	if (m_rHealthBarRectangle.right <= 0.0 && !m_pMainHero->IsDead()) {
		m_pMainHero->PlayAnimationOnceAndStopTrack("dead");
		m_pMainHero->SetDead(true);

		pEnemy->SetAnimationOnTrack("idle", 0);
	}
	if (m_rEnemyHealthBarRectangle.right <= 0.0 && !pEnemy->IsDead()) {
		pEnemy->PlayAnimationOnceAndStopTrack("dead");
		pEnemy->SetDead(true);
	}

	for (auto& gameObject : pApp->GetGameObjManager()->GetSkinnedModels()) {
		//main hero attacking enemy
		if (//pDinput->IsMouseButtonDown(0) &&
			IsObjectNear(m_pMainHero->GetPosition(), gameObject->GetPosition()) &&
			m_pMainHero->GetName() != gameObject->GetName() &&
			gameObject->GetActorType() == "enemy" &&
			!m_pMainHero->IsDead()
			) {
			m_bIsEnemyHealthBarVisible = true;
			gameObject->SetAttacked(true);
			gameObject->SetAttackerName(m_pMainHero->GetName());

			int num = rand() % 2;
			LPCSTR animName = num % 2 == 0 ? "attack_1" : "attack_2";
			m_pMainHero->PlayAnimationOnce(animName);
			if (m_pMainHero->JustStartedPlayingAnimationOnce()) {
				m_rEnemyHealthBarRectangle.right -= 70;
			}
		}

		// main hero attacked by enemy
		if (gameObject->IsAttacked() && 
			IsObjectNear(m_pMainHero->GetPosition(), gameObject->GetPosition()) &&
			!gameObject->IsDead() && 
			!m_pMainHero->IsDead()) {
			if (m_rHealthBarRectangle.right > 0.0) {
				m_pMainHero->SetAttackerName(gameObject->GetName());
				auto pSkinnedModel = std::static_pointer_cast<SkinnedModel>(gameObject);

				int num = rand() % 2;
				LPCSTR animName = num % 2 == 0 ? "attack_1" : "attack_2";
				pSkinnedModel->PlayAnimationOnce(animName);

				if (gameObject->JustStartedPlayingAnimationOnce()) {
					m_rHealthBarRectangle.right -= 70;
				}
			}
			else {
				gameObject->SetAttacked(false);
			}
		}

		//when the enemy is attacked it updates his rotation so it can face the mainHero
		if (gameObject->IsAttacked() && 
			!gameObject->IsDead() && 
			IsObjectNear(m_pMainHero->GetPosition(), gameObject->GetPosition())) {
			D3DXVECTOR3 vActorPosition = gameObject->GetPosition();
			D3DXVECTOR3 vMainHeroPosition = m_pMainHero->GetPosition();

			D3DXVECTOR3 vDistanceVector = vActorPosition - vMainHeroPosition;
			D3DXVec3Normalize(&vDistanceVector, &vDistanceVector);

			//for some reason the look std::vector is the right std::vector must be fixed
			float angle = D3DXVec3Dot(&gameObject->GetRightVector(), &vDistanceVector);
			gameObject->ModifyRotationAngleByY(angle);

			D3DXMATRIX R;
			D3DXMatrixRotationY(&R, angle);
			gameObject->TransformByMatrix(R);
		}


		//if mainHero is fighting with enemy, but started to run and is no longer close to the enemy, 
		//the enemy updates his vectors so he can face the mainHero and run in his direction.
		//When he is close enough he start to attack again.
		if (gameObject->IsAttacked() && 
			!gameObject->IsDead() && 
			!IsObjectNear(m_pMainHero->GetPosition(), gameObject->GetPosition()) &&
			!m_pMainHero->IsDead()
			) {
			RunToTarget(gameObject, m_pMainHero->GetPosition(), dt);
		}
	}
}

/////////////////////////////////////////////////////////////////////////

void Game::RunToTarget(std::shared_ptr<SkinnedModel> runner, D3DXVECTOR3 targetPos, float dt) {
	float speed = 400.f;
	runner->SetMovementSpeed(speed / 100);
	runner->SetAnimationSpeed(speed / 100);
	runner->PlayAnimation("run");

	D3DXVECTOR3 dir(0.0f, 0.0f, 0.0f);

	dir -= runner->GetLookVector();

	D3DXVECTOR3 newPos = runner->GetPosition() + dir * speed*dt;
	runner->SetPosition(newPos);

	runner->AlignToDirection(targetPos);
}

/////////////////////////////////////////////////////////////////////////

//makes the camera to follow model in game and moving it with WASD from keyboard and mouse
void Game::MoveActor(std::shared_ptr<SkinnedModel> actor, float dt) {
	//this std::vector holds the new direction to move
	D3DXVECTOR3 dir(0.0f, 0.0f, 0.0f);
	
	if (pApp->GetDinput()->IsKeyDown(DIK_W) ) {
		actor->PlayAnimation("run");
		dir += m_pCamera->GetLookVector();
	}
	if (pApp->GetDinput()->IsKeyDown(DIK_S) ) {
		actor->PlayAnimation("run");
		dir -= m_pCamera->GetLookVector();
	}
	if (pApp->GetDinput()->IsKeyDown(DIK_A) ) {
		actor->PlayAnimation("run");
		dir -= m_pCamera->GetRightVector();
	}
	if (pApp->GetDinput()->IsKeyDown(DIK_D) ) {
		actor->PlayAnimation("run");
		dir += m_pCamera->GetRightVector();
	}

	if (!pApp->GetDinput()->IsKeyDown(DIK_W) && 
		!pApp->GetDinput()->IsKeyDown(DIK_S) && 
		!pApp->GetDinput()->IsKeyDown(DIK_A) && 
		!pApp->GetDinput()->IsKeyDown(DIK_D)) {
		actor->PlayAnimation("idle");
	}
	
	if (m_pCamera->GetCameraMode() == ECameraMode::MoveWithoutPressedMouse) {
		//if we just move the mouse move the camera
		RotateActor(actor, dt);
	}
	else if (m_pCamera->GetCameraMode() == ECameraMode::MoveWithPressedMouse) {
		//if we hold the left mouse button move the camera
		if (pApp->GetDinput()->IsMouseButtonDown(0)) {
			RotateActor(actor, dt);
		}
	}

	D3DXVECTOR3 newPos = actor->GetPosition() + dir*150.0* actor->GetMovementSpeed()*dt;
	if(m_pTerrain->IsValidPosition(newPos.x,newPos.z)) {
		actor->SetPosition(newPos);
	}

	//updates the camera position based on the new position of the model and the zoom
	//we zoom in the direction of the look std::vector. If the zoom is negative it will go in the opposite direction
	D3DXVECTOR3 upOffset = D3DXVECTOR3(0, 25, 0);
	D3DXVECTOR3 cameraPos = actor->GetPosition() + m_pCamera->GetLookVector()* m_pCamera->GetZoom() + upOffset;

	m_pCamera->SetPosition(cameraPos);
}

/////////////////////////////////////////////////////////////////////////

//rotates to model and the camera if the mouse is moved
void Game::RotateActor(std::shared_ptr<SkinnedModel> actor, float dt) {
	float yAngle = pApp->GetDinput()->GetMouseDX() / (19000*dt);
	actor->ModifyRotationAngleByY(yAngle);
	
	D3DXMATRIX R;
	D3DXMatrixRotationY(&R, yAngle);

	m_pCamera->TransformByMatrix(R);

	actor->GetLookVector()  = m_pCamera->GetLookVector();
	actor->GetRightVector() = m_pCamera->GetRightVector();
	actor->GetUpVector()    = m_pCamera->GetUpVector();
}

/////////////////////////////////////////////////////////////////////////

void Game::OnRender() {
	pApp->GetDevice()->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0);
	pApp->GetDevice()->BeginScene();
	
			m_pSky->OnRender(m_pCamera);
			m_pTerrain->OnRender(m_pCamera);

			for (auto& gameObject : pApp->GetGameObjManager()->GetGameObjects()) {
				gameObject->OnRender(m_pCamera);
				//DrawLine(gameObject->GetPosition(), gameObject->GetLookVector());
				//DrawLine(gameObject->GetPosition(), gameObject->GetUpVector());
				//DrawLine(gameObject->GetPosition(), gameObject->GetRightVector());
			}
			
			//DrawLine(m_pCamera->GetPosition(), m_pCamera->GetLookVector());

			auto BB = m_pMainHero->GetBB();
			BB = BB.TransformByMatrix(BB.m_transformationMatrix);
			for (auto& object: pApp->GetGameObjManager()->GetGameObjects()) {
				auto BB1 = object->GetBB();
				BB1 = BB1.TransformByMatrix(BB1.m_transformationMatrix);

				if (m_pMainHero->GetName().compare(object->GetName())) {
					if (BB.Collide(BB1)) {
						//cout << "COLLIDING" << object->GetName() << endl;
					}
				}
			}

			pApp->GetTextManager()->DrawFPS();
	
			//draws dialogues
			for (auto& dialogue : m_pDialogueManager->GetDialogues()) {
				m_pDialogueManager->RenderDialogueTree(dialogue->m_pTree->m_pRoot);
			}

			//text->drawText("Press L to switch between the two camera modes",400,40,0,0,255,0,0,0);

			//draws quest stuff
			for(auto& quest: GetQuestManager()->GetQuests()) {
				if( quest->m_bIsStarted ) {
					if (quest->m_bIsCompleted) {
						pApp->GetTextManager()->RenderText(quest->m_strTitle.c_str(), pApp->GetPresentParameters().BackBufferWidth - 420, 70, 0, 0, 255, 255, 255, 0);
						pApp->GetTextManager()->RenderText("Completed", pApp->GetPresentParameters().BackBufferWidth - 590, 70, 0, 0, 255, 255, 255, 0);
					}
					else if (!m_pMainHero->IsDead()) {
						pApp->GetTextManager()->RenderText(quest->m_strTitle.c_str(), pApp->GetPresentParameters().BackBufferWidth - 420, 70, 0, 0, 255, 255, 255, 0);
					}
					else if (!quest->m_bIsCompleted && m_pMainHero->IsDead()) {
						pApp->GetTextManager()->RenderText(quest->m_strTitle.c_str(), pApp->GetPresentParameters().BackBufferWidth - 420, 70, 0, 0, 255, 255, 255, 0);
						pApp->GetTextManager()->RenderText("Epic fail!", pApp->GetPresentParameters().BackBufferWidth - 570, 70, 0, 0, 255, 255, 255, 0);
					}
				}
			}

			//draws the healthbars
			m_pInterfaceSprite->Begin(D3DXSPRITE_ALPHABLEND);

				m_pInterfaceSprite->Draw(m_pHealthBarTexture,NULL,NULL,&m_vHealthBarPosition,D3DXCOLOR(255,255,255,255));
				m_pInterfaceSprite->Draw(m_pHealthBarFilledTexture,&m_rHealthBarRectangle,NULL,&m_vHealthBarPosition,D3DXCOLOR(255,255,255,255));

				if( m_bIsEnemyHealthBarVisible ) {
					m_pInterfaceSprite->Draw(m_pHealthBarTexture,NULL,NULL,&m_vEnemyHealthBarPosition,D3DXCOLOR(255,255,255,255));
					m_pInterfaceSprite->Draw(m_phealthBarFilledEnemyTexture,&m_rEnemyHealthBarRectangle,NULL,&m_vEnemyHealthBarPosition,D3DXCOLOR(255,255,255,255));
				}

			m_pInterfaceSprite->End();
			
			m_pHealSpell->OnRender();

			m_pGunEffect->OnRender(m_pCamera);

			m_pWater->OnRender(m_pCamera);

	pApp->GetDevice()->EndScene();

	pApp->GetDevice()->Present(0, 0, 0, 0);
}
	
/////////////////////////////////////////////////////////////////////////

void Game::DrawLine(const D3DXVECTOR3& vStart, const D3DXVECTOR3& vEnd) {
	//pApp->GetDevice()->BeginScene();

	m_pDebugGraphicsEffect->SetTechnique(m_hDebugGraphicsTechnique);

	D3DXMATRIX viewProjMatrix = m_pCamera->GetViewProjMatrix();
	m_pDebugGraphicsEffect->SetMatrix(m_hDebugGraphicsWVPMatrix, &viewProjMatrix);
	UINT numPasses = 0;
	m_pDebugGraphicsEffect->Begin(&numPasses, 0);
	m_pDebugGraphicsEffect->BeginPass(0);

		IDirect3DVertexBuffer9* pVertexBuffer;

		CheckSuccess(pApp->GetDevice()->CreateVertexBuffer(2 * sizeof (VertexPositionColor), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &pVertexBuffer, 0));

		VertexPositionColor* v = 0;
		pVertexBuffer->Lock(0, 0,  (void**)&v, 0);

		v[0].m_pos = vStart;
		//v[0].m_color = D3DXCOLOR(1.0f,1.0,1.0f,1.0f);

		v[1].m_pos = vEnd;
		//v[1].m_color = D3DXCOLOR(1.0f,1.0,1.0f,1.0f);

		pVertexBuffer->Unlock();

		pApp->GetDevice()->SetStreamSource(0, pVertexBuffer, 0, sizeof(VertexPositionColor));

		pApp->GetDevice()->SetVertexDeclaration(pApp->GetPCDecl());

		pApp->GetDevice()->DrawPrimitive(D3DPT_LINELIST,0,1);

	m_pDebugGraphicsEffect->EndPass();
	m_pDebugGraphicsEffect->End();

	//pDxDevice->EndScene();

	//pDxDevice->Present(0, 0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////

//checks if two models are close
//i think this should be changed in the future with circles
bool Game::IsObjectNear(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2, float t) {
	if((pos1.x > pos2.x-t) && (pos1.x < pos2.x+t) &&
	   (pos1.z > pos2.z-t) && (pos1.z < pos2.z+t)) {
	   return true;
	}

	return false;
}

Game::~Game() {
	lua_close(m_pLuaState);
}

/////////////////////////////////////////////////////////////////////////

void Game::OnLostDevice() {
	m_pSky->OnLostDevice();
	pApp->GetTextManager()->OnLostDevice();
	m_pTerrain->OnLostDevice();

	for (auto& gameObject : pApp->GetGameObjManager()->GetGameObjects()) {
		gameObject->OnLostDevice();
	}

	m_pInterfaceSprite->OnLostDevice();
	m_pHealSpell->OnLostDevice();
	m_pWater->OnLostDevice();
}

/////////////////////////////////////////////////////////////////////////

void Game::OnResetDevice() {
	m_pSky->OnResetDevice();
	pApp->GetTextManager()->OnResetDevice();
	m_pTerrain->OnResetDevice();

	for (auto& gameObject : pApp->GetGameObjManager()->GetGameObjects()) {
		gameObject->OnResetDevice();
	}
	m_pInterfaceSprite->OnResetDevice();

	m_pHealSpell->OnResetDevice();

	//after the onResetDevice there might be change in the size of the screen so set
	//the new dimensions to the camera
	//float w = (float)pApp->GetPresentParameters().BackBufferWidth;
	//float h = (float)pApp->GetPresentParameters().BackBufferHeight;
	//camera->BuildProjectionMatrix(D3DX_PI * 0.25f, w/h, 1.0f, 2000.0f);

	m_pWater->OnResetDevice();
}

/////////////////////////////////////////////////////////////////////////

LRESULT Game::MsgProc(UINT msg, WPARAM wParam, LPARAM lParam) {
	switch( msg ) {
		//Sent when the window is activated or deactivated.(pressed alt+tab)
		//Game is paused when the window is inactive, and unpaused when become active again
		case WM_ACTIVATE: {
			if( LOWORD(wParam) == WA_INACTIVE ) {
				pApp->SetPaused(true);
			}
			else {
				pApp->SetPaused(false);
			}

			return 0;
		}

		//Sent when the user closes the window
		case WM_CLOSE: {
			DestroyWindow( pApp->GetMainWindow() );
			return 0;
		}

		//Sent when the window is destroyed
		case WM_DESTROY:{
			PostQuitMessage(0);
			return 0;
		}

		case WM_KEYDOWN: {
			switch(wParam) {
				case 'L': {
					if(m_pCamera->GetCameraMode() == ECameraMode::MoveWithoutPressedMouse ) {
						m_pCamera->SetCameraMode(ECameraMode::MoveWithPressedMouse);
					}
					else if(m_pCamera->GetCameraMode() == ECameraMode::MoveWithPressedMouse ) {
						m_pCamera->SetCameraMode(ECameraMode::MoveWithoutPressedMouse);
					}

					break;
				}
				case 'B': {
					pApp->GetGameObjManager()->SetShouldRenderBoundingBoxes(!pApp->GetGameObjManager()->ShouldRenderBoundingBoxes());

					break;
				}
				case 'C': {
					//TODO: if the camera becomes attached again align it with the object
					m_pCamera->SetCameraFree(!m_pCamera->IsCameraFree());

					break;
				}
				case 'Q': {
					m_rHealthBarRectangle.right += 20;

					break;
				}

				case VK_SHIFT: {
					m_pMainHero->SetMovementSpeed(2);
					break;
				}
			}

			return 0;
		}

		case WM_MOUSEWHEEL: {
			auto delta = GET_WHEEL_DELTA_WPARAM(wParam);
			m_pCamera->ModifyZoom(delta / 10);
			return 0;
		}

		case WM_LBUTTONDOWN:{
			switch (wParam) {
				case MK_LBUTTON:{
					pApp->GetGameObjManager()->UpdatePicking(m_pCamera);
				}
				break;
			}
			return 0;
		}

		case WM_KEYUP: {
			switch( wParam ) {
				case VK_SHIFT: {
					m_pMainHero->SetMovementSpeed(1);
					break;
				}
			}
		}
	}

	return DefWindowProc(pApp->GetMainWindow(), msg, wParam, lParam);
}


/////////////////////////////////////////////////////////////////////////
