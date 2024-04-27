#include "./h2bParser.h"
#include "./userInterface.h"
#include <chrono>
#include "playerStats.h"

// Creation, Rendering & Cleanup
class RendererManager
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GOpenGLSurface ogl;
	GW::MATH::GMatrix gMatrixProxy;	
	GW::INPUT::GController gController;
	GW::INPUT::GInput gInput;

	//for camera
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF cameraMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
	bool reset = false;
	GW::MATH::GVECTORF mainMenuCamPos;
	GW::MATH::GVECTORF mainMenuLookAtPos;

	//for ui
	GW::MATH::GMATRIXF UIviewMatrix;
	GW::MATH::GMATRIXF UIcameraMatrix;
	GW::MATH::GMATRIXF UIorthoMatrix;

	Application *app;
	GameConfig* gameConfig;

	//create level
	Level_Objects* lvl;

	//Global variables for key inputs
	bool tab;
	bool t;
	bool leftMouse;


public:

	//globals to track is mainmenu or pause hud was enabled
	bool isMainMenuRendered;
	bool isPauseMenuRendered;

	bool freecam = false;
	//ui panels
	playerUi* playerHUD;
	mainMenuUi* mainMenuHUD;
	pauseMenuUi* pauseMenu;
	treasureMenuUi* treasureMenu;
	controlsMenuUi* controlsMenu;
	gameOverUi* gameOverMenu;
	std::vector <uiPanel*> panels;
	

	RendererManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl, GameConfig& _gameConfig, Application &application, Level_Objects& Level)
	{
		
		//passed arguments for initializing
		//gInput = &_gInput;
		gameConfig = &_gameConfig;
		win = _win;
		ogl = _ogl;
		app = &application;
		lvl = &Level;

		//sets default state for menu keybinds
		tab = false;
		t = false;
		leftMouse = false;

		isMainMenuRendered = false;
		isPauseMenuRendered = false;

		mainMenuCamPos = {
		gameConfig->at("MainMenuCameraPos").at("posx").as<float>(),
		gameConfig->at("MainMenuCameraPos").at("posy").as<float>(),
		gameConfig->at("MainMenuCameraPos").at("posz").as<float>(),
		gameConfig->at("MainMenuCameraPos").at("posw").as<float>() };

		mainMenuLookAtPos = {
		gameConfig->at("MainMenuCameraPos").at("lookx").as<float>(),
		gameConfig->at("MainMenuCameraPos").at("looky").as<float>(),
		gameConfig->at("MainMenuCameraPos").at("lookz").as<float>(),
		gameConfig->at("MainMenuCameraPos").at("lookw").as<float>() };

		//sets default state for freecam
		freecam = true;

		GW::SYSTEM::GLog log;

		log.Create("output.txt");

		//load ui Panels - this doesn't turn them on but simply lay out each UI for rendering later on.
		initializePanels(log);

		////PANELS/////
		//pauseMenu->toggleRender();
		mainMenuHUD->toggleRender();
		//playerHUD->toggleRender();
		//treasureMenu->toggleRender();
		//controlsMenu->toggleRender();
		//gameOverMenu->toggleRender();
	
		lvl->UploadLevelToGPU(ogl, cameraMatrix, viewMatrix, projectionMatrix);

		//create inputs
		gController.Create();
		gInput.Create(win);

		//initialize camera and view matrix based on position and look at
		GW::MATH::GVECTORF camPos = { 0.0f, 5.0, 10.0,1.0 };
		GW::MATH::GVECTORF lookAtPos = { 0.0f, 2.0f, 0.0f, 1.0f };
		cameraMatrix = initializeCamView(camPos, lookAtPos);

		//initialize projection matrix based on FOV and near and far planes
		projectionMatrix = initializeProjectionMatrix(_ogl, 65.0f, 0.1f, 100.0f);

		//UI matricies - not currently used
		UIorthoMatrix = initializeOrthoprojectionMatrix();
		gMatrixProxy.IdentityF(UIcameraMatrix);
		gMatrixProxy.InverseF(UIcameraMatrix, UIviewMatrix);
	}

	//initializes all panels
	void initializePanels(GW::SYSTEM::GLog& log) {
		//assign the panels to the preset renderManager pointers
		playerUi* player = new playerUi(*gameConfig);
		playerHUD = player;

		mainMenuUi* main = new mainMenuUi(*gameConfig);
		mainMenuHUD = main;

		pauseMenuUi* pause = new pauseMenuUi(*gameConfig);
		pauseMenu = pause;

		treasureMenuUi* treasure = new treasureMenuUi(*gameConfig);
		treasureMenu = treasure;

		controlsMenuUi* controls = new controlsMenuUi(*gameConfig);
		controlsMenu = controls;

		gameOverUi* gameOver = new gameOverUi(*gameConfig);
		gameOverMenu = gameOver;

		//Load All meshes in the level at start
		bool playerHUDSuccess = playerHUD->LoadMeshes("../playerHUD.txt", "../Models/playerHUDModels", log.Relinquish());
		bool mainMenuHUDSuccess = mainMenuHUD->LoadMeshes("../MainMenuHUD.txt", "../Models/MainMenuHUDmodels", log.Relinquish());
		bool pauseMenuSuccess = pauseMenu->LoadMeshes("../PauseMenu.txt", "../Models/PauseMenuModels", log.Relinquish());
		bool treasureMenuSuccess = treasureMenu->LoadMeshes("../treasureMenu.txt", "../Models/treasureMenuModels", log.Relinquish());
		bool controlsMenuSuccess = controlsMenu->LoadMeshes("../controlsMenu.txt", "../Models/controlsMenuModels", log.Relinquish());
		bool gameOverMenuSuccess =  gameOverMenu->LoadMeshes("../GameOver.txt", "../Models/gameOverModels", log.Relinquish());

		//add to vector of panels
		panels.push_back(playerHUD);
		panels.push_back(mainMenuHUD);
		panels.push_back(pauseMenu);
		panels.push_back(treasureMenu);
		panels.push_back(controlsMenu);
		panels.push_back(gameOverMenu);

		for (uiPanel* panel : panels) {
			initializePanel(panel);
		}
	}

	//initialize single panel
	void initializePanel(uiPanel* panel) {
		panel->assign();
		panel->arrange();
		panel->start();
		panel->UploadLevelToGPU(UIcameraMatrix, UIviewMatrix, UIorthoMatrix);
	}

	//initializes a world matrix and sets it to identity
	GW::MATH::GMATRIXF initializeWorldMatrix()
	{
		//create identity matrix as world Matrix
		GW::MATH::GMATRIXF worldMatrix;
		GW::MATH::GMatrix::IdentityF(worldMatrix);

		return worldMatrix;
	}

	//Intializes the view Matrix, and returns the camera matrix based on position and look at
	GW::MATH::GMATRIXF initializeCamView(GW::MATH::GVECTORF cameraPosition, GW::MATH::GVECTORF lookAt)
	{
		//create identity matrix as world Matrix
		GW::MATH::GMATRIXF camMatrix;			//initialize to identity
		GW::MATH::GMatrix::IdentityF(camMatrix);

		//Create View
		GW::MATH::GVECTORF upVector = { 0.0f, 1.0f, 0.0f, 0.0f };
		GW::MATH::GMatrix::LookAtRHF(cameraPosition, lookAt, upVector, viewMatrix);

		//Get camera matrix
		GW::MATH::GMatrix::InverseF(viewMatrix, camMatrix);

		return camMatrix;
	}

	//initializes projection matrix and returns one
	GW::MATH::GMATRIXF initializeProjectionMatrix(GW::GRAPHICS::GOpenGLSurface _ogl, float degrees, float _near, float _far)
	{
		GW::MATH::GMATRIXF projMatrix;
		GW::MATH::GMatrix::IdentityF(projMatrix);

		float aspectRatio;
		_ogl.GetAspectRatio(aspectRatio);

		GW::MATH::GMatrix::ProjectionOpenGLRHF(toRad(degrees), aspectRatio, _near, _far, projMatrix);

		return projMatrix;
	}

	//initializes an Orthoprojectionmatrix
	GW::MATH::GMATRIXF initializeOrthoprojectionMatrix()
	{
		float _left = 0.0;
		float _right = gameConfig->at("Window").at("width").as<int>();
		float _bottom = 0.0;
		float _top = gameConfig->at("Window").at("height").as<int>();
		float _near = -1;
		float _far = 1;

		// Create the orthographic projection matrix
		GW::MATH::GMATRIXF ortho = {
			2 / (_right - _left),   0,                      0,                  -(_right + _left) / (_right - _left),
			0,                      2 / (_top - _bottom),   0,                  -(_top + _bottom) / (_top - _bottom),
			0,                      0,                      -2 / (_far - _near), -(_far + _near) / (_far - _near),
			0,                      0,                      0,                  1
		};

		return ortho;
	}
	//Updates camera movement based on movement
	void UpdateCamera(int windowWidth, int windowHeight)
	{
		//previous call time
		static std::chrono::high_resolution_clock::time_point callTime = std::chrono::high_resolution_clock::now();

		//for keystates
		float space, lShift, rTrigger, lTrigger, wKey, sKey, aKey, dKey, lStickY, lStickX, rStickX, rStickY, mouseX, mouseY, num0;

		//camera variables
		float totalYChange, totalZChange, totalXChange, thumbspeed, FOV, pitch, yaw;

		// current time
		std::chrono::high_resolution_clock::time_point currTime = std::chrono::high_resolution_clock::now();

		//get the time passed
		std::chrono::duration<float> updateTime = currTime - callTime;

		//Change Settings
		gInput.GetState(G_KEY_NUMPAD_0, num0);
		if (num0 > 0)
		{
			if (reset == false)
			{
				freecam = !freecam;
				reset = true;
			}
		}
		else
		{
			reset = false;
		}

		GW::MATH::GMATRIXF rotationMatrix;

		//Keyboard / mouse
		gInput.GetState(G_KEY_SPACE, space);
		gInput.GetState(G_KEY_LEFTSHIFT, lShift);
		gInput.GetState(G_KEY_W, wKey);
		gInput.GetState(G_KEY_S, sKey);
		gInput.GetState(G_KEY_A, aKey);
		gInput.GetState(G_KEY_D, dKey);
		GW::GReturn mouse = gInput.GetMouseDelta(mouseX, mouseY);

		//controller
		GW::GReturn controller = gController.GetState(0, G_RIGHT_TRIGGER_AXIS, rTrigger);
		gController.GetState(0, G_LEFT_TRIGGER_AXIS, lTrigger);
		gController.GetState(0, G_LY_AXIS, lStickY);
		gController.GetState(0, G_LX_AXIS, lStickX);
		gController.GetState(0, G_RY_AXIS, rStickY);
		gController.GetState(0, G_RX_AXIS, rStickX);

		if (freecam){

			if (controller != GW::GReturn::FAILURE)
			{
				//Calculate total change
				totalYChange = space - lShift + rTrigger - lTrigger;
				totalZChange = wKey - sKey + lStickY;
				totalXChange = dKey - aKey + lStickX;

				//calculate rotation
				thumbspeed = G_PI * updateTime.count();
				FOV = toRad(65.0f);
				pitch = (FOV * mouseY) / windowHeight + rStickY * (-thumbspeed);
				yaw = (FOV * windowWidth / windowHeight * mouseX) / windowWidth + rStickX * thumbspeed;
			}

			else
			{
				//Calculate total change
				totalYChange = space - lShift;
				totalZChange = wKey - sKey;
				totalXChange = dKey - aKey;

				//calculate rotation
				thumbspeed = G_PI * updateTime.count();
				FOV = toRad(65.0f);
				pitch = (FOV * mouseY) / windowHeight;
				yaw = (FOV * windowWidth / windowHeight * mouseX) / windowWidth;
			}
		
		
			//calculate translation
			const float Camera_Speed = 10 * 0.5f;
			float perFrameSpeed = Camera_Speed * updateTime.count();
			float cameraPositionY = totalYChange * perFrameSpeed;
			float cameraPositionZ = -totalZChange * perFrameSpeed;
			float cameraPositionX = totalXChange * perFrameSpeed;

			//GW::MATH::GMatrix::IdentityF(rotationMatrix);
			GW::MATH::GMatrix::InverseF(viewMatrix, rotationMatrix);

			//check if mouse value is redundant - if so do nothing
			if (mouse != GW::GReturn::REDUNDANT && mouse == GW::GReturn::SUCCESS)
			{
				// do nothing

				//gMatrixProxy.RotationYawPitchRollF(-yaw, -pitch, 0.0f, rotationMatrix);

				gMatrixProxy.RotateXLocalF(rotationMatrix, -pitch, rotationMatrix);
				gMatrixProxy.RotateYGlobalF(rotationMatrix, -yaw, rotationMatrix);
			}

			//if value is redundant, set mouseX and mouseY to zero to prevent drift
			else
			{
				mouseX = 0;
				mouseY = 0;
			}

			//Translation vector
			GW::MATH::GVECTORF cameraTranslationVector = { cameraPositionX, cameraPositionY, cameraPositionZ, 1.0f };

			//apply translation to the camera
			gMatrixProxy.TranslateLocalF(rotationMatrix, cameraTranslationVector, rotationMatrix);

			//apply rotation 
			//gMatrixProxy.MultiplyMatrixF(rotationMatrix, cameraMatrix, cameraMatrix);
	
		}

		//Freeze the camera in main menu
		else if (mainMenuHUD->render)
		{
			cameraMatrix = initializeCamView(mainMenuCamPos, mainMenuLookAtPos);
		}
		
		else // not freecam
		{
			Model m;
			m.name = "MegaBee";

			auto found = std::find(lvl->allObjectsInLevel.begin(), lvl->allObjectsInLevel.end(), m);

			if (found != lvl->allObjectsInLevel.end())
			{
				size_t index = found - lvl->allObjectsInLevel.begin();
				m.world = lvl->allObjectsInLevel[index].world;

				gMatrixProxy.InverseF(rotationMatrix, rotationMatrix);
				gMatrixProxy.IdentityF(rotationMatrix);
				auto r = gMatrixProxy.LookAtRHF(GW::MATH::GVECTORF{ m.world.row4.x, m.world.row4.y + 10, m.world.row4.z, 1 }, 
												GW::MATH::GVECTORF{ m.world.row4.x, m.world.row4.y, m.world.row4.z + .0001f, 1 },
												GW::MATH::GVECTORF{ 0, 1, 0, 0 }, rotationMatrix);
				gMatrixProxy.InverseF(rotationMatrix, rotationMatrix);
			}
		}

		//get view matrix
		gMatrixProxy.InverseF(rotationMatrix, viewMatrix);

		callTime = currTime;
	}

	//Event Handling for all buttons - manually place each button here and tag the lamda expression it should execute
	void eventHandling() {

		//Return Left Mouse state for re-use
		if ((leftMouse) && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
			leftMouse = false;
		}

		//MAINMENU
		if (mainMenuHUD->render) {

			if (!leftMouse && mainMenuHUD->controlsButton->HandleInputLeftMouseButton(gInput)) {
				leftMouse = true;
				isMainMenuRendered = true;
				controlsMenu->render = true;
				mainMenuHUD->render = false;

			}

			//mainMenuHUD->startButton->HandleInput(mainMenuHUD->startButton, G_BUTTON_LEFT, gInput, turnOffRender);
			mainMenuHUD->exitButton->HandleInput(app, G_BUTTON_LEFT, gInput, shutdown);
		}

		//PAUSEMENU
		else if (pauseMenu->render){

			if (!leftMouse && pauseMenu->controlsPauseMenuButton->HandleInputLeftMouseButton(gInput)) {
				
				isPauseMenuRendered = true;
				controlsMenu->render = true;
				pauseMenu->render = false;

			}

			pauseMenu->restartPauseMenuButton->HandleInput(pauseMenu->restartPauseMenuButton, G_BUTTON_LEFT, gInput, turnOffRender);
			pauseMenu->exitPauseMenuButton->HandleInput(app, G_BUTTON_LEFT, gInput, shutdown);
			pauseMenu->resumePauseMenuButton->HandleInput(dynamic_cast<uiPanel*>(pauseMenu), G_BUTTON_LEFT, gInput, turnOffPanel);

		}

		//CONTROLSMENU
		else if (controlsMenu->render){

			if (!leftMouse && controlsMenu->exitControlsMenuButton->HandleInputLeftMouseButton(gInput)) {

				leftMouse = true;
				controlsMenu->render = false;

				if (isMainMenuRendered) {
					mainMenuHUD->render = true;
					isMainMenuRendered = false;
				}

				else if (isPauseMenuRendered) {
					pauseMenu->render = true;
					isPauseMenuRendered = false;
				}
			}


		}

		else if (gameOverMenu->render) {
			gameOverMenu->exitGameOverButton->HandleInput(app, G_BUTTON_LEFT, gInput, shutdown);
		}

	
		//TREASURE MENU
		else if (treasureMenu->render){
			treasureMenu->exitTreasureMenuButton->HandleInput(dynamic_cast<uiPanel*>(treasureMenu), G_BUTTON_LEFT, gInput, turnOffPanel);
		}
	
		//KEYBINDS: Everything here should be checking if there was a key pressed and performing some action after
		//CONTROLS:
		//			TAB: toggles pause menu
		//			  T: toggles treasure menu
		//
		//
		//

		{	//TOGGLE PAUSE MENU
			if (!tab && (GetAsyncKeyState(VK_TAB) & 0x8000)) {
				if (!pauseMenu->render && !mainMenuHUD->render && !treasureMenu->render && !controlsMenu->render) {
					pauseMenu->render = true;

				}

				else if (pauseMenu->render) {
					pauseMenu->render = false;
				}
				tab = true;

			}
			else if (tab && !(GetAsyncKeyState(VK_TAB) & 0x8000)) {
				tab = false;
			}

		}

		{	//TOGGLE PAUSE MENU
			if (!t &&(GetAsyncKeyState(0x54) & 0x8000)) {
				if (!pauseMenu->render && !mainMenuHUD->render && !treasureMenu->render && !controlsMenu->render) {
					treasureMenu->render = true;
					
				}

				else if (treasureMenu->render) {
					treasureMenu->render = false;
				}
				t = true;
				
			}
			else if (t && !(GetAsyncKeyState(0x54) & 0x8000)) {
				t = false;
			}
			
		}

	}

	//Render Loop for all objects (place Panels and Levels here);
	void Render(){		
		lvl->Render(cameraMatrix, viewMatrix, projectionMatrix);

		for (uiPanel* panel : panels){

			if (panel == playerHUD && panel->render)
			{
				playerHUD->Render(UIcameraMatrix, UIviewMatrix, UIorthoMatrix);
			}

			else if (panel == gameOverMenu && panel->render)
			{
				gameOverMenu->Render(UIcameraMatrix, UIviewMatrix, UIorthoMatrix);
			}
			else if (panel->render){
				panel->Render(UIcameraMatrix, UIviewMatrix, UIorthoMatrix);
			}
		}

		eventHandling();
	}

	//swaps the level in render manager
	void changeLevel(Level_Objects& level, int levelNum) {
		level.UploadLevelToGPU(ogl, cameraMatrix, viewMatrix, projectionMatrix);
		if(playerHUD->render)
			playerHUD->updateLevelText(levelNum);
		lvl = &level;
	}

	~RendererManager() {
		for (uiPanel* panel : panels)
		{
			delete panel;
		}
	}


};

struct Models { Model mod; };

void AddEntities(std::shared_ptr <Level_Objects> Level, std::shared_ptr<flecs::world> game)
{
	for each (Model i in Level->allObjectsInLevel)
	{
		auto e = game->entity(i.name.c_str());
		e.set<DD::Name>({ i.name });
		e.set<DD::World>({ i.world });
		//e.add<DD::World>();
		e.set<Models>({ i });

		if (i.name.substr(0, 5) == "alien")
			e.add<DD::Enemy>();

		if (i.name.substr(0, 9) != "RealFloor")
			e.set<DD::Collidable>({ i.obb });


		// Add debug output to verify OBBs are being added
		//std::cout << "OBB added for entity: " << i.name << std::endl;

		if (i.name == "MegaBee")
		{
			e.add<DD::Player>();
		}

		//if (i.name == "alien")
		//{
		//	e.add<DD::Enemy>();
		//	e.add<DD::Health>();
		//}
	}
}


void UpdatePlayerScore(RendererManager& rm, PlayerStats& ps, std::shared_ptr<GameConfig> gc) {
	std::cout << "success!" << std::endl;

		ps.updateScore(50);
		rm.playerHUD->updateHUDScore(ps.getScore());

		if (ps.getScore() > (*gc).at("Player1").at("highscore").as<int>())
		{
			(*gc)["Player1"]["highscore"] = ps.getScore();
			rm.playerHUD->updateHUDHighScore(ps.getScore());
		}
}


void AddSystems(std::shared_ptr<Level_Objects> level,
	std::shared_ptr<flecs::world> game,
	std::shared_ptr<GameConfig> gameConfig,
	GW::INPUT::GInput immediateInput,
	GW::INPUT::GBufferedInput bufferedInput,
	GW::INPUT::GController controllerInput,
	GW::AUDIO::GAudio _audioEngine,
	GW::CORE::GEventGenerator eventPusher, PlayerStats* ps, RendererManager* rm){

	flecs::system playerSystem;

	std::shared_ptr<const GameConfig> readCfg = gameConfig;
	float speed = (*readCfg).at("Player1").at("speed").as<float>();
	float bullSpeed = (*readCfg).at("Lazers").at("speed").as<float>();

	flecs::system playerShootSystem = game->system<DD::Player, DD::World>("Player Shoot System")
		.iter([immediateInput, game, level, bullSpeed](flecs::iter it, DD::Player*, DD::World* world)
			{
				for (auto i : it)
				{
					static bool U = false, D = false, L = false, R = false;
					float input = 0, shootUp = 0, shootDown = 0, shootLeft = 0, shootRight = 0;

					GW::INPUT::GInput t = immediateInput;
					if (!U && (GetAsyncKeyState(VK_UP) & 0x8000))
					{
						shootUp = 1;
						U = true;
					}
					else if (U && !(GetAsyncKeyState(VK_UP) & 0x8000))
						U = false;

					if (!D && (GetAsyncKeyState(VK_DOWN) & 0x8000))
					{
						shootDown = 1;
						D = true;
					}
					else if (D && !(GetAsyncKeyState(VK_DOWN) & 0x8000))
						D = false;

					if (!L && (GetAsyncKeyState(VK_LEFT) & 0x8000))
					{
						shootLeft = 1;
						L = true;
					}
					else if (L && !(GetAsyncKeyState(VK_LEFT) & 0x8000))
						L = false;

					if (!R && (GetAsyncKeyState(VK_RIGHT) & 0x8000))
					{
						shootRight = 1;
						R = true;
					}
					else if (R && !(GetAsyncKeyState(VK_RIGHT) & 0x8000))
						R = false;

					int shootState = 0;

					if (shootUp > 0)
						shootState = 1;
					if (shootLeft > 0)
						shootState = 2;
					if (shootRight > 0)
						shootState = 3;
					if (shootDown > 0)
						shootState = 4;

					int index = 0;
					Model modelToDupe;
					//GW::MATH::GMATRIXF world{};
					for each (Model m in level->allObjectsInLevel)
					{
						if (m.name == "BeeStinger")
						{
							modelToDupe = m;
							//world = m.world;
							break;
						}
						index++;
					}

					std::string count;
					auto f = game->filter<DD::CountBullet>();
					count = std::to_string(f.count());
					/*std::cout << count << std::endl;*/
					switch (shootState)
					{
					case 1:
					{
						auto tempEnt = game->entity(count.c_str());
						tempEnt.add<DD::CountBullet>();

						modelToDupe.world = world->value;
						modelToDupe.name += count;
						level->allObjectsInLevel.push_back(modelToDupe);
						auto e = game->entity(modelToDupe.name.c_str());
						e.set<Models>({ modelToDupe });
						e.set<DD::Collidable>({ modelToDupe.obb });
						e.set<DD::World>({ world->value });
						e.set<DD::Name>({ modelToDupe.name });
						e.set<DD::BulletVel>({ GW::MATH::GVECTORF{0, 0, bullSpeed } });
						e.add<DD::Bullet>();

						break;
					}

					case 2:
					{
						auto tempEnt = game->entity(count.c_str());
						tempEnt.add<DD::CountBullet>();

						modelToDupe.world = world->value;
						modelToDupe.name += count;
						level->allObjectsInLevel.push_back(modelToDupe);
						auto e = game->entity(modelToDupe.name.c_str());
						e.set<Models>({ modelToDupe });
						e.set<DD::Collidable>({ modelToDupe.obb });
						e.set<DD::World>({ world->value });
						e.set<DD::Name>({ modelToDupe.name });
						e.set<DD::BulletVel>({ GW::MATH::GVECTORF{-bullSpeed, 0, 0 } });
						e.add<DD::Bullet>();
						break;
					}

					case 3:
					{
						auto tempEnt = game->entity(count.c_str());
						tempEnt.add<DD::CountBullet>();

						modelToDupe.world = world->value;
						modelToDupe.name += count;
						level->allObjectsInLevel.push_back(modelToDupe);
						auto e = game->entity(modelToDupe.name.c_str());
						e.set<Models>({ modelToDupe });
						e.set<DD::Collidable>({ modelToDupe.obb });
						e.set<DD::World>({ world->value });
						e.set<DD::Name>({ modelToDupe.name });
						e.set<DD::BulletVel>({ GW::MATH::GVECTORF{bullSpeed, 0, 0 } });
						e.add<DD::Bullet>();
						break;
					}

					case 4:
					{
						auto tempEnt = game->entity(count.c_str());
						tempEnt.add<DD::CountBullet>();

						modelToDupe.world = world->value;
						modelToDupe.name += count;
						level->allObjectsInLevel.push_back(modelToDupe);
						auto e = game->entity(modelToDupe.name.c_str());
						e.set<Models>({ modelToDupe });
						e.set<DD::Collidable>({ modelToDupe.obb });
						e.set<DD::World>({ world->value });
						e.set<DD::Name>({ modelToDupe.name });
						e.set<DD::BulletVel>({ GW::MATH::GVECTORF{0, 0, -bullSpeed } });
						e.add<DD::Bullet>();
						break;
					}

					case 0:
						break;
					}
				}
			});

	flecs::system bulletSystem = game->system<DD::Bullet>("Bullet System")
		.each([level, rm, ps, &gameConfig](flecs::entity arrow, DD::Bullet)
			{
				// damage anything we come into contact with
				arrow.each<DD::CollidedWith>([&arrow, level, rm, ps, &gameConfig](flecs::entity hit)
					{
						if (!(hit.has<DD::Player>() || hit.has<DD::Bullet>()))
						{
							Model m = hit.get<Models>()->mod;
							if (hit.has <DD::Enemy>())
							{
								auto found = std::find(level->allObjectsInLevel.begin(), level->allObjectsInLevel.end(), m);

								if (found != level->allObjectsInLevel.end())
								{
									level->allObjectsInLevel.erase(found);
								}
								hit.destruct();
								UpdatePlayerScore(*rm, *ps, gameConfig); // update score if we hit an enemy
							}
							m = arrow.get<Models>()->mod;
							auto found = std::find(level->allObjectsInLevel.begin(), level->allObjectsInLevel.end(), m);

							if (found != level->allObjectsInLevel.end())
							{
								level->allObjectsInLevel.erase(found);
								
							}							
							arrow.destruct();
						}
						
					});
			});

	flecs::system playerCollisionSystem = game->system<DD::Player>("Player Collision System")
		.each([level](flecs::entity pl, DD::Player)
			{
				pl.each<DD::CollidedWith>([&pl, level](flecs::entity hit)
					{

						if (!(hit.has<DD::Bullet>() || hit.has<DD::Enemy>()))
						{
							//std::cout << hit.get<DD::Name>()->name << std::endl;

							//pl.set<DD::World>({pl.get<DD::LastWorld>()->value});
							hit.remove<DD::CollidedWith>();
						}
						//pl.remove<DD::CollidedWith>();
					});

			});
	//flecs::system enemyCollisionSystem = game->system<DD::Enemy>("Enemy Collision System")
	//	.each([level](flecs::entity pl, DD::Enemy)
	//		{
	//			pl.each<DD::CollidedWith>([&pl, level](flecs::entity hit)
	//				{
	//					if (!(hit.has<DD::Bullet>()))
	//					{
	//						hit.remove<DD::CollidedWith>();
	//						
	//					}
	//					pl.destruct();
	//				});

	//		});

	flecs::system bulletMove = game->system<DD::BulletVel, DD::World, DD::Name, Models>("Bullet Move System")
		.iter([immediateInput, game, level, bullSpeed](flecs::iter it, DD::BulletVel* v, DD::World* w, DD::Name* n, Models* m)
			{
				for (auto i : it)
				{
					auto found = std::find(level->allObjectsInLevel.begin(), level->allObjectsInLevel.end(), m[i].mod);

					if (found != level->allObjectsInLevel.end())
					{
						size_t index = found - level->allObjectsInLevel.begin();

						GW::MATH::GVECTORF moveVec = { v[i].value.x * it.delta_time() * bullSpeed, 0, v[i].value.z * it.delta_time() * bullSpeed };
						auto e = game->lookup(n[i].name.c_str());
						DD::World* edit = game->entity(e).get_mut<DD::World>();
						GW::MATH::GMatrix::TranslateLocalF(edit->value, moveVec, edit->value);

						level->allObjectsInLevel[index].world = edit->value;
					}
				}
			});

}

