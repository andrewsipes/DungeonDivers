#include "./h2bParser.h"
#include "./userInterface.h"
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

	//for defaults
	GameConfig* gameConfig;

	//create level
	Level_Objects lvl;

	//ui panels
	playerUi playerHUD;
	std::vector <uiPanel> panels;


public:

	RendererManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl, GameConfig& _gameConfig)
	{
		//assigns the gameConfig for passing
		gameConfig = &_gameConfig;

		GW::SYSTEM::GLog log;
		log.Create("output.txt");
		bool levelSuccess = lvl.LoadMeshes("../GameLevel.txt", "../Models", log.Relinquish(), ogl, cameraMatrix, viewMatrix, projectionMatrix);
		
		playerUi player(gameConfig);
		playerHUD = player;

		panels.push_back(playerHUD);
		bool playerHUDSuccess = playerHUD.LoadMeshes("../ui.txt", "../Models/uiModels", log.Relinquish());
	
		win = _win;
		ogl = _ogl;

		//create inputs
		gController.Create();
		gInput.Create(win);

		//initialize camera and view matrix based on position and look at
		GW::MATH::GVECTORF camPos = { 0.0f, 5.0, 10.0,1.0 };
		GW::MATH::GVECTORF lookAtPos = { 0.0f, 2.0f, 0.0f, 1.0f };
		cameraMatrix = initializeCamView(camPos, lookAtPos);

		//initialize projection matrix based on FOV and near and far planes
		projectionMatrix = initializeProjectionMatrix(_ogl, 65.0f, 0.1f, 100.0f);
	
		lvl.UploadLevelToGPU(ogl, cameraMatrix, viewMatrix, projectionMatrix);

		playerHUD.toggleRender();
		playerHUD.assign();
		playerHUD.arrange();
		playerHUD.start();
		playerHUD.UploadLevelToGPU();


		
	}



	//initializes panels to default state
	void initializePanels() {

		for (uiPanel panel : panels) {
			panel.assign();
			panel.arrange();
			panel.start();
			panel.UploadLevelToGPU();
		}
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
		if (mouse != GW::GReturn::REDUNDANT && mouse == GW::GReturn::SUCCESS)
		{
			// do nothing
		}

		//if value is redundant, set mouseX and mouseY to zero to prevent drift
		else
		{
			mouseX = 0;
			mouseY = 0;
		}


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

	//Render Loop for all objects (place Panels and Levels here);
	void Render()
	{

		lvl.Render(ogl, cameraMatrix, viewMatrix, projectionMatrix);
		playerHUD.Render();
		
		playerHUD.heart1->HandleInput(*playerHUD.heart1, gInput);


	}

	~RendererManager()
	{
	}
};
