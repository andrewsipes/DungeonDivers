#include "./h2bParser.h"
#include "./userInterface.h"
#include <chrono>

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
	bool reset = false;
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF cameraMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
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

	//gloals to track is mainmenu or pause hud was enabled
	bool isMainMenuRendered;
	bool isPauseMenuRendered;

public:

	//ui panels
	playerUi* playerHUD;
	mainMenuUi* mainMenuHUD;
	pauseMenuUi* pauseMenu;
	treasureMenuUi* treasureMenu;
	controlsMenuUi* controlsMenu;
	std::vector <uiPanel*> panels;
	
	bool freecam;

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

		//Load All meshes in the level at start
		bool playerHUDSuccess = playerHUD->LoadMeshes("../playerHUD.txt", "../Models/playerHUDModels", log.Relinquish());
		bool mainMenuHUDSuccess = mainMenuHUD->LoadMeshes("../MainMenuHUD.txt", "../Models/MainMenuHUDmodels", log.Relinquish());
		bool pauseMenuSuccess = pauseMenu->LoadMeshes("../PauseMenu.txt", "../Models/PauseMenuModels", log.Relinquish());
		bool treasureMenuSuccess = treasureMenu->LoadMeshes("../treasureMenu.txt", "../Models/treasureMenuModels", log.Relinquish());
		bool controlsMenuSuccess = controlsMenu->LoadMeshes("../controlsMenu.txt", "../Models/controlsMenuModels", log.Relinquish());

		//add to vector of panels
		panels.push_back(playerHUD);
		panels.push_back(mainMenuHUD);
		panels.push_back(pauseMenu);
		panels.push_back(treasureMenu);
		panels.push_back(controlsMenu);

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

		
		}

		//Freeze the camera in main menu
		else if (mainMenuHUD->render)
		{
			cameraMatrix = initializeCamView(mainMenuCamPos, mainMenuLookAtPos);
		}

		else {
			gMatrixProxy.InverseF(cameraMatrix, cameraMatrix);
			gMatrixProxy.IdentityF(cameraMatrix);
			auto r = gMatrixProxy.LookAtRHF(GW::MATH::GVECTORF{ 1.826, 13.327, 1.267, 1 }, GW::MATH::GVECTORF{ 1.826, 12.327, 1.2671, 1 }, GW::MATH::GVECTORF{ 0, 1, 0, 0 }, cameraMatrix);
			gMatrixProxy.InverseF(cameraMatrix, cameraMatrix);
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

			mainMenuHUD->startButton->HandleInput(mainMenuHUD->startButton, G_BUTTON_LEFT, gInput, turnOffRender);
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
			else if (panel->render){
				panel->Render(UIcameraMatrix, UIviewMatrix, UIorthoMatrix);
			}
		}

		eventHandling();
	}

	//swaps the level in render manager
	void changeLevel(Level_Objects& level) {
		level.UploadLevelToGPU(ogl, cameraMatrix, viewMatrix, projectionMatrix);
		lvl = &level;
	}

	~RendererManager() {
		for (uiPanel* panel : panels)
		{
			delete panel;
		}
	}


};

