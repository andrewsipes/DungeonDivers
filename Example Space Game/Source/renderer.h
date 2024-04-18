#include "./h2bParser.h"
#include "./userInterface.h"


//LAMBDA FUNCTIONS
//Place all Ui Related button calls here for now


// Creation, Rendering & Cleanup
class RendererManager
{

	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GOpenGLSurface ogl;
	GW::MATH::GMatrix gMatrixProxy;
	GW::INPUT::GInput gInput;
	GW::INPUT::GController gController;

	//for camera
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF cameraMatrix;
	GW::MATH::GMATRIXF projectionMatrix;

	//for ui
	GW::MATH::GMATRIXF UIviewMatrix;
	GW::MATH::GMATRIXF UIcameraMatrix;
	GW::MATH::GMATRIXF UIorthoMatrix;

	Application *app;
	GameConfig* gameConfig;
	Level_Objects lvl;

	//Global variables for key inputs
	bool tab = false;
	bool t = false;

public:
	//ui panels
	playerUi* playerHUD;
	mainMenuUi* mainMenuHUD;
	pauseMenuUi* pauseMenu;
	treasureMenuUi* treasureMenu;
	std::vector <uiPanel*> panels;

	RendererManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl, GameConfig& _gameConfig, Application &application)
	{
		//passed arguments for initializing
		gameConfig = &_gameConfig;
		win = _win;
		ogl = _ogl;
		app = &application;

		//sets default state for pausing the game
		tab = false;
		t = false;

		GW::SYSTEM::GLog log;
		log.Create("output.txt");

		//load ui Panels - this doesn't turn them on but simply lay out each UI for rendering later on.
		initializePanels(log);

		//Toggle which level you want to load
		{
			/////LEVELS/////
			//bool levelSuccess = lvl.LoadMeshes("../GameLevel.txt", "../Models", log.Relinquish(), ogl, cameraMatrix, viewMatrix, projectionMatrix);
			bool levelSuccess = lvl.LoadMeshes("../MainMenu.txt", "../Models/MainMenuModels", log.Relinquish());


			////PANELS/////
			//pauseMenu->toggleRender();
			//mainMenuHUD->toggleRender();
			//playerHUD->toggleRender();
			//treasureMenu->toggleRender();
		}
		


		lvl.UploadLevelToGPU(ogl, cameraMatrix, viewMatrix, projectionMatrix);

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
	void initializePanels(GW::SYSTEM::GLog &log) {

		//assign the panels to the preset renderManager pointers
		playerUi* player = new playerUi(*gameConfig);
		playerHUD = player;

		mainMenuUi* main = new mainMenuUi(*gameConfig);
		mainMenuHUD = main;

		pauseMenuUi* pause = new pauseMenuUi(*gameConfig);
		pauseMenu = pause;

		treasureMenuUi* treasure = new treasureMenuUi(*gameConfig);
		treasureMenu = treasure;

		//Load All meshes in the level at start
		bool playerHUDSuccess = playerHUD->LoadMeshes("../playerHUD.txt", "../Models/playerHUDModels", log.Relinquish());
		bool mainMenuHUDSuccess = mainMenuHUD->LoadMeshes("../MainMenuHUD.txt", "../Models/MainMenuHUDmodels", log.Relinquish());
		bool pauseMenuSuccess = pauseMenu->LoadMeshes("../PauseMenu.txt", "../Models/PauseMenuModels", log.Relinquish());
		bool treasureMenuSuccess = treasureMenu->LoadMeshes("../treasureMenu.txt", "../Models/treasureMenuModels", log.Relinquish());


		//add to vector of panels
		panels.push_back(playerHUD);
		panels.push_back(mainMenuHUD);
		panels.push_back(pauseMenu);
		panels.push_back(treasureMenu);

		for (uiPanel* panel : panels) {
			initializePanel(panel);
		}
	}

	//initialize single panel
	void initializePanel(uiPanel *panel) {
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
		float space, lShift, rTrigger, lTrigger, wKey, sKey, aKey, dKey, lStickY, lStickX, rStickX, rStickY, mouseX, mouseY;

		//camera variables
		float totalYChange, totalZChange, totalXChange, thumbspeed, FOV, pitch,yaw;

		// current time
		std::chrono::high_resolution_clock::time_point currTime = std::chrono::high_resolution_clock::now();

		//get the time passed
		std::chrono::duration<float> updateTime = currTime - callTime;

		//Keyboard / mouse
		gInput.GetState(G_KEY_SPACE, space);
		gInput.GetState(G_KEY_LEFTSHIFT, lShift);
		gInput.GetState(G_KEY_W, wKey);
		gInput.GetState(G_KEY_S, sKey);
		gInput.GetState(G_KEY_A, aKey);
		gInput.GetState(G_KEY_D, dKey);
		GW::GReturn mouse = gInput.GetMouseDelta(mouseX, mouseY);

		//controller
		GW:: GReturn controller = gController.GetState(0, G_RIGHT_TRIGGER_AXIS, rTrigger);
		gController.GetState(0, G_LEFT_TRIGGER_AXIS, lTrigger);
		gController.GetState(0, G_LY_AXIS, lStickY);
		gController.GetState(0, G_LX_AXIS, lStickX);
		gController.GetState(0, G_RY_AXIS, rStickY);
		gController.GetState(0, G_RX_AXIS, rStickX);

		//check if mouse value is redundant - if so do nothing
		if (mouse != GW::GReturn::REDUNDANT && mouse == GW::GReturn::SUCCESS){
			// do nothing
		}

		//if value is redundant, set mouseX and mouseY to zero to prevent drift
		else{
			mouseX = 0;
			mouseY = 0;
		}


		if (controller != GW::GReturn::FAILURE){
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

		else{
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

		//create rotation matrix
		GW::MATH::GMATRIXF rotationMatrix;
		GW::MATH::GMatrix::IdentityF(rotationMatrix);

		gMatrixProxy.RotationYawPitchRollF(-yaw, -pitch, 0.0f, rotationMatrix);

		//Translation vector
		GW::MATH::GVECTORF cameraTranslationVector = { cameraPositionX, cameraPositionY, cameraPositionZ, 1.0f };

		//apply translation to the camera
		gMatrixProxy.TranslateLocalF(cameraMatrix, cameraTranslationVector, cameraMatrix);

		//apply rotation 
		gMatrixProxy.MultiplyMatrixF(rotationMatrix, cameraMatrix, cameraMatrix);
	
		//get view matrix
		gMatrixProxy.InverseF(cameraMatrix, viewMatrix);

		//update call time for next calc
		callTime = currTime;
	}

	//Event Handling for all buttons - manually place each button here and tag the lamda expression it should execute
	void eventHandling() {

		//MAINMENU
		if (mainMenuHUD->render) {
			mainMenuHUD->startButton->HandleInput(mainMenuHUD->startButton, G_BUTTON_LEFT, gInput, turnOffRender);
			mainMenuHUD->controlsButton->HandleInput(mainMenuHUD->controlsButton, G_BUTTON_LEFT, gInput, turnOffRender);
			mainMenuHUD->exitButton->HandleInput(app, G_BUTTON_LEFT, gInput, shutdown);
		}

		//PLAYERHUD
		if (playerHUD->render) {

		}

		//PAUSEMENU
		if (pauseMenu->render){

			pauseMenu->controlsPauseMenuButton->HandleInput(pauseMenu->controlsPauseMenuButton, G_BUTTON_LEFT, gInput, turnOffRender);
			pauseMenu->restartPauseMenuButton->HandleInput(pauseMenu->restartPauseMenuButton, G_BUTTON_LEFT, gInput, turnOffRender);
			pauseMenu->exitPauseMenuButton->HandleInput(app, G_BUTTON_LEFT, gInput, shutdown);

			if (pauseMenu->resumePauseMenuButton->HandleInputBool(G_BUTTON_LEFT, gInput) == true) {
				//resume game
				pauseMenu->toggleRender();
			}

		}

		if (treasureMenu->render)
		{
			if (treasureMenu->exitTreasureMenuButton->HandleInputBool(G_BUTTON_LEFT, gInput) == true)
			{
				treasureMenu->toggleRender();
			}
		}

		//KEYBINDS: Everything here should be checking if there was a key pressed and performing some action after
		//CONTROLS:
		//			TAB: toggles pause menu
		//			  T: toggles treasure menu
		//
		//
		//

		
		{	//TOGGLE PAUSE MENU
			if ((GetAsyncKeyState(VK_TAB) & 0x8000) && !tab) {
				if (!pauseMenu->render && !mainMenuHUD->render && !treasureMenu->render) {
					pauseMenu->render = true;
				}

				else if (pauseMenu->render) {
					pauseMenu->render = false;
				}
				tab = true;
			}
			else if (!(GetAsyncKeyState(VK_TAB) & 0x8000)) {
				tab = false;
			}
		}

		{	//TOGGLE PAUSE MENU
			if ((GetAsyncKeyState(0x54) & 0x8000) && !t) {
				if (!pauseMenu->render && !mainMenuHUD->render && !treasureMenu->render) {
					treasureMenu->render = true;
					
				}

				else if (treasureMenu->render) {
					treasureMenu->render = false;
				}
				t = true;
				
			}
			else if (!(GetAsyncKeyState(0x54) & 0x8000)) {
				t = false;
			}
			
		}

		

	

	}

	//Render Loop for all objects (place Panels and Levels here);
	void Render(){		
		lvl.Render(cameraMatrix, viewMatrix, projectionMatrix);

		for (uiPanel* panel : panels){

			if (panel == treasureMenu && panel->render){
				treasureMenu->Render(UIcameraMatrix, UIviewMatrix, UIorthoMatrix);
			}

			if (panel->render){
				panel->Render(UIcameraMatrix, UIviewMatrix, UIorthoMatrix);
			}
		}

		eventHandling();
	
	}

	~RendererManager(){
		for (uiPanel* panel : panels)
		{
			delete panel;
		}
	}


};

