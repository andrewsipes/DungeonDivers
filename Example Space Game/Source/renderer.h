#include "./h2bParser.h"
#include "./load_object_oriented.h"

// Creation, Rendering & Cleanup
class RendererManager
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GOpenGLSurface ogl;
	GW::MATH::GMatrix gMatrixProxy;
	GW::INPUT::GInput gInput;
	GW::INPUT::GController gController;
	UBO_DATA ubo;

	unsigned int screenHeight;
	unsigned int screenWidth;

	//for camera
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF cameraMatrix;
	GW::MATH::GMATRIXF projectionMatrix;

	//create level
	Level_Objects lvl;

public:
	RendererManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl)
	{
		GW::SYSTEM::GLog log;
		log.Create("output.txt");
		bool success = lvl.LoadLevel("../GameLevel.txt", "../Models", log.Relinquish(), ogl, cameraMatrix, viewMatrix, projectionMatrix);
	
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
	//void UpdateCamera(int windowWidth, int windowHeight)
	//{
	//	//previous call time
	//	static std::chrono::high_resolution_clock::time_point callTime = std::chrono::high_resolution_clock::now();
	//	//for keystates
	//	float space, lShift, rTrigger, lTrigger, wKey, sKey, aKey, dKey, lStickY, lStickX, rStickX, rStickY, mouseX, mouseY;
	//	//camera variables
	//	float totalYChange, totalZChange, totalXChange, thumbspeed, FOV, pitch,yaw;
	//	// current time
	//	std::chrono::high_resolution_clock::time_point currTime = std::chrono::high_resolution_clock::now();
	//	//get the time passed
	//	std::chrono::duration<float> updateTime = currTime - callTime;
	//	//Keyboard / mouse
	//	gInput.GetState(G_KEY_SPACE, space);
	//	gInput.GetState(G_KEY_LEFTSHIFT, lShift);
	//	gInput.GetState(G_KEY_W, wKey);
	//	gInput.GetState(G_KEY_S, sKey);
	//	gInput.GetState(G_KEY_A, aKey);
	//	gInput.GetState(G_KEY_D, dKey);
	//	GW::GReturn mouse = gInput.GetMouseDelta(mouseX, mouseY);
	//	//controller
	//	GW:: GReturn controller = gController.GetState(0, G_RIGHT_TRIGGER_AXIS, rTrigger);
	//	gController.GetState(0, G_LEFT_TRIGGER_AXIS, lTrigger);
	//	gController.GetState(0, G_LY_AXIS, lStickY);
	//	gController.GetState(0, G_LX_AXIS, lStickX);
	//	gController.GetState(0, G_RY_AXIS, rStickY);
	//	gController.GetState(0, G_RX_AXIS, rStickX);
	//	////check if mouse value is redundant - if so do nothing
	//	//if (mouse != GW::GReturn::REDUNDANT && mouse == GW::GReturn::SUCCESS)
	//	//{
	//	//	// do nothing
	//	//}
	//	////if value is redundant, set mouseX and mouseY to zero to prevent drift
	//	//else
	//	//{
	//	//	mouseX = 0;
	//	//	mouseY = 0;
	//	//}
	//	//if (controller != GW::GReturn::FAILURE)
	//	//{
	//	//	//Calculate total change
	//	//	totalYChange = space - lShift + rTrigger - lTrigger;
	//	//	totalZChange = wKey - sKey + lStickY;
	//	//	totalXChange = dKey - aKey + lStickX;
	//	//	//calculate rotation
	//	//	thumbspeed = G_PI * updateTime.count();
	//	//	FOV = toRad(65.0f);
	//	//	pitch = (FOV * mouseY) / windowHeight + rStickY * (-thumbspeed);
	//	//	yaw = (FOV * mouseX) / windowWidth + rStickX * thumbspeed;
	//	//}
	//	//else
	//	//{
	//	//	//Calculate total change
	//	//	totalYChange = space - lShift;
	//	//	totalZChange = wKey - sKey;
	//	//	totalXChange = dKey - aKey;
	//	//	//calculate rotation
	//	//	thumbspeed = G_PI * updateTime.count();
	//	//	FOV = toRad(65.0f);
	//	//	pitch = (FOV * mouseY) / windowHeight;
	//	//	yaw = (FOV * windowWidth / windowHeight * mouseX) / windowWidth;
	//	//}
	//	//calculate translation
	//	const float Camera_Speed = 10 * 0.5f;
	//	float perFrameSpeed = Camera_Speed * updateTime.count();
	//	float cameraPositionY = 0.0f;
	//		cameraPositionY += totalYChange* perFrameSpeed;
	//	float cameraPositionZ = 0.0f;
	//		cameraPositionZ -= -totalZChange * perFrameSpeed;
	//	float cameraPositionX = 0.0f;
	//		cameraPositionX += totalXChange * perFrameSpeed;
	//	//create rotation matrix
	//	//GW::MATH::GMATRIXF rotationMatrix;
	//	//GW::MATH::GMatrix::IdentityF(rotationMatrix);
	//	//gMatrixProxy.RotationYawPitchRollF(-yaw, -pitch, 0.0f, rotationMatrix);
	//	////Translation vector
	//	//GW::MATH::GVECTORF cameraTranslationVector = { cameraPositionX, cameraPositionY, cameraPositionZ, 1.0f };
	//	////apply translation to the camera
	//	//gMatrixProxy.TranslateLocalF(cameraMatrix, cameraTranslationVector, cameraMatrix);
	//	////apply rotation 
	//	//gMatrixProxy.MultiplyMatrixF(rotationMatrix, cameraMatrix, cameraMatrix);
	//
	//	////get view matrix
	//	//gMatrixProxy.InverseF(cameraMatrix, viewMatrix);
	//	//copy of the current view matrix
	//	GW::MATH::GMATRIXF originalViewMatrix;
	//	//places the view matrix temporarily in world space by taking its inverse
	//	GW::MATH::GMatrix::InverseF(ubo._view, originalViewMatrix);
	//	if (mouse != GW::GReturn::REDUNDANT || rStickX != 0.0f || rStickY != 0.0f)
	//	{
	//		GW::MATH::GMATRIXF newPitchMatrix;
	//		gMatrixProxy.IdentityF(newPitchMatrix);
	//		GW::MATH::GMATRIXF newYawMatrix;
	//		gMatrixProxy.IdentityF(newPitchMatrix);
	//		gMatrixProxy.RotateXLocalF(originalViewMatrix, -pitch, newPitchMatrix);
	//		gMatrixProxy.RotateYGlobalF(newPitchMatrix, -yaw, newYawMatrix);
	//		//matrixProxy.RotateXLocalF(originalViewMatrix, -totalPitch, originalViewMatrix);
	//		//matrixProxy.RotateYLocalF(originalViewMatrix, -totalYaw, originalViewMatrix);
	//		originalViewMatrix = newYawMatrix;
	//	}
	//	//update call time for next calc
	//	callTime = currTime;
	//}

void UpdateCamera()
{
	//ogl.GetAspectRatio(aspectRatio);

	// Measure time between calls using std::chrono
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
	lastTime = currentTime;

	// Calculate PerFrameSpeed
	float cameraSpeed = 3.0f; //ramped up for quick movement
	float perFrameSpeed = cameraSpeed * deltaTime;

	float combinedYMovement = 0.0f;
	float combinedXMovement = 0.0f;
	float combinedZMovement = 0.0f;

	float thumbSpeed = G2D_PI * deltaTime;
	float mouseYDelta = 0.0f;
	float mouseXDelta = 0.0f;
	float fieldOfView = G_DEGREE_TO_RADIAN_F(65.0f);

	GW::GReturn deltaChange = gInput.GetMouseDelta(mouseXDelta, mouseYDelta);

	//key and button variables for camera movement
	float spaceKeyState = 0.0f;
	float leftShiftKeyState = 0.0f;
	float wKeyState = 0.0f;
	float sKeyState = 0.0f;
	float dKeyState = 0.0f;
	float aKeyState = 0.0f;
	float num1KeyState = 0.0f;
	float num2KeyState = 0.0f;
	float num3KeyState = 0.0f;
	float leftArrowKeyState = 0.0f;
	float rightArrowKeyState = 0.0f;

	float rightTriggerState = 0.0f;
	float leftTriggerState = 0.0f;
	float leftStickXAxis = 0.0f;
	float leftStickYAxis = 0.0f;
	float rightStickXAxis = 0.0f;
	float rightStickYAxis = 0.0f;

	//keys
	gInput.GetState(G_KEY_LEFTSHIFT, leftShiftKeyState);
	gInput.GetState(G_KEY_SPACE, spaceKeyState);
	gInput.GetState(G_KEY_W, wKeyState);
	gInput.GetState(G_KEY_S, sKeyState);
	gInput.GetState(G_KEY_D, dKeyState);
	gInput.GetState(G_KEY_A, aKeyState);
	gInput.GetState(G_KEY_1, num1KeyState);
	gInput.GetState(G_KEY_2, num2KeyState);
	gInput.GetState(G_KEY_3, num3KeyState);
	gInput.GetState(G_KEY_LEFT, leftArrowKeyState);
	gInput.GetState(G_KEY_RIGHT, rightArrowKeyState);

	//buttons
	gController.GetState(0, G_RIGHT_TRIGGER_AXIS, rightTriggerState);
	gController.GetState(0, G_LEFT_TRIGGER_AXIS, leftTriggerState);
	gController.GetState(0, G_RX_AXIS, rightStickXAxis);
	gController.GetState(0, G_RY_AXIS, rightStickYAxis);
	gController.GetState(0, G_LX_AXIS, leftStickXAxis);
	gController.GetState(0, G_LY_AXIS, leftStickYAxis);

	//total X and Y change based on input states
	float totalZChange = wKeyState - sKeyState + leftStickYAxis;
	combinedZMovement -= totalZChange * perFrameSpeed;

	float totalXChange = dKeyState - aKeyState + leftStickXAxis;
	combinedXMovement += totalXChange * perFrameSpeed;

	float totalYChange = spaceKeyState - leftShiftKeyState + rightTriggerState - leftTriggerState;
	combinedYMovement += totalYChange * perFrameSpeed;

	float totalPitch = fieldOfView * mouseYDelta / screenHeight + rightStickYAxis * -thumbSpeed;
	float totalYaw = fieldOfView * mouseXDelta / screenWidth + rightStickXAxis * thumbSpeed;

	if (num1KeyState != 0.0f) // If key 1 is pressed - cam 1
	{
		GW::MATH::GVECTORF cameraPosition2 = { -4.0f, 6.0f, -4.0f, 1.0f }; //high up by entrance
		GW::MATH::GVECTORF lookingAt2 = { 1.0f, 1.5f, 0.0f, 1.0f };
		GW::MATH::GVECTORF lookingUp2 = { 0.0f, 1.0f, 0.0f, 0.0f };

		//View matrix setup for camera 1
		gMatrixProxy.IdentityF(ubo._view);
		gMatrixProxy.LookAtRHF(cameraPosition2, lookingAt2, lookingUp2, ubo._view);
	}
	else if (num2KeyState != 0.0f) // If key 2 is pressed - cam 2
	{
		GW::MATH::GVECTORF cameraPosition1 = { 3.0f, 1.5f, 0.0f, 1.0f }; //by locked exit
		GW::MATH::GVECTORF lookingAt1 = { 1.0f, 1.5f, 0.0f, 1.0f };
		GW::MATH::GVECTORF lookingUp1 = { 0.0f, 1.0f, 0.0f, 0.0f };

		//View matrix setup for camera 2
		gMatrixProxy.IdentityF(ubo._view);
		gMatrixProxy.LookAtRHF(cameraPosition1, lookingAt1, lookingUp1, ubo._view);
	}
	else if (num3KeyState != 0.0f) // If key 3 is pressed - go back to original camera
	{
		GW::MATH::GVECTORF originalCamPos = { -9.0f, 1.5f, 0.0f, 1.0f }; //outside of entrance
		GW::MATH::GVECTORF lookingAt = { 1.0f, 1.5f, 0.0f, 1.0f };
		GW::MATH::GVECTORF lookingUp = { 0.0f, 1.0f, 0.0f, 0.0f };

		//View matrix setup for main camera
		gMatrixProxy.IdentityF(ubo._view);
		gMatrixProxy.LookAtRHF(originalCamPos, lookingAt, lookingUp, ubo._view);
	}

	//copy of the current view matrix
	GW::MATH::GMATRIXF originalViewMatrix;
	//places the view matrix temporarily in world space by taking its inverse
	GW::MATH::GMatrix::InverseF(ubo._view, originalViewMatrix);

	if (deltaChange != GW::GReturn::REDUNDANT || rightStickXAxis != 0.0f || rightStickYAxis != 0.0f)
	{
		GW::MATH::GMATRIXF newPitchMatrix;
		gMatrixProxy.IdentityF(newPitchMatrix);

		GW::MATH::GMATRIXF newYawMatrix;
		gMatrixProxy.IdentityF(newPitchMatrix);

		gMatrixProxy.RotateXLocalF(originalViewMatrix, -totalPitch, newPitchMatrix);
		gMatrixProxy.RotateYGlobalF(newPitchMatrix, -totalYaw, newYawMatrix);

		//matrixProxy.RotateXLocalF(originalViewMatrix, -totalPitch, originalViewMatrix);
		//matrixProxy.RotateYLocalF(originalViewMatrix, -totalYaw, originalViewMatrix);

		originalViewMatrix = newYawMatrix;
	}

	//create a translation vector
	GW::MATH::GVECTORF translationVector = { combinedXMovement, combinedYMovement, combinedZMovement, 0.0 };
	//apply translation directly to viewMatrix
	GW::MATH::GMatrix::TranslateLocalF(originalViewMatrix, translationVector, originalViewMatrix);

	//take the inverse
	gMatrixProxy.InverseF(originalViewMatrix, ubo._view);
}

	void Render()
	{
		lvl.RenderLevel(ogl, cameraMatrix, viewMatrix, projectionMatrix);
	}

	~RendererManager()
	{
		lvl.UnloadLevel();
	}
};
