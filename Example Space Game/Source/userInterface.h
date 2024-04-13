#include "./load_object_oriented.h"
#include <algorithm>

//This entire .H file handles the UserInterface Classes, and their accompanying methods
//In order to use this properly, objects should be created in render.h and handled as needed for rendering, events, etc.

//uiModel is a derivate of the original model with updated definitions specific to UI
class uiModel : public Model
{

public:
	bool render;

	GameConfig* gameConfig;

	uiModel()
	{
		render = false;
	}

	void SetUpPipeline() override{

		glUseProgram(shaderExecutable);
		glBindVertexArray(vertexArray);
		SetVertexAttributes();
		
		bool isUi = true;
		glUniform1i(glGetUniformLocation(shaderExecutable, "isUi"), isUi);
	}

	void updateUniformBufferObject(const H2B::MATERIAL _material) {

		glBindBuffer(GL_UNIFORM_BUFFER, UBOBufferObject);
		ubo = updateUboInstance(_material);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ubo), &ubo);
	}

	//assigns ubo data to send to the shader
	UBO_DATA updateUboInstance(H2B::MATERIAL _material){

		UBO_DATA _ubo;

		_ubo._world = world;

		//material
		_ubo.material.Kd = _material.attrib.Kd;
		_ubo.material.d = _material.attrib.d;
		_ubo.material.illum = _material.attrib.illum;
		_ubo.material.Ka = _material.attrib.Ka;
		_ubo.material.Kd = _material.attrib.Kd;
		_ubo.material.Ke = _material.attrib.Ke;
		_ubo.material.Ks = _material.attrib.Ks;
		_ubo.material.Ni = _material.attrib.Ni;
		_ubo.material.Ns = _material.attrib.Ns;
		_ubo.material.sharpness = _material.attrib.sharpness;
		_ubo.material.Tf = _material.attrib.Tf;

		return _ubo;
	}

	bool DrawModel(){

		//Get Block Index, and Bind the Buffer
		int blockIndex = (glGetUniformBlockIndex(shaderExecutable, "UboData"));
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOBufferObject);
		glUniformBlockBinding(shaderExecutable, blockIndex, 0);

		updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));

		//sets hud in front of everything else
		glDepthRange(0.0, 0.05);

		//Draw meshes - iterates through the meshes and materials to draw them individually.
		for (int j = 0; j < cpuModel.meshCount; j++) {
			updateUniformBufferObject(cpuModel.materials[cpuModel.meshes[j].materialIndex]);
			SetUpPipeline();
			updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));
			glDrawElements(GL_TRIANGLES, cpuModel.meshes[j].drawInfo.indexCount, GL_UNSIGNED_INT, (void*)(cpuModel.meshes[j].drawInfo.indexOffset * sizeof(cpuModel.indices[0])));
		}

		glBindVertexArray(0);
		glUseProgram(0);

		return true;

	}

	bool UploadModelData2GPU() {

	
		ubo = updateUboInstance(cpuModel.materials[0]);

		InitializeGraphics();


		return true;

	}

	void toggleRender() {
		render = !render;
	}

	// Scales a model's vertices
	void scale(float scale) {

		//Retrived height and width of the window to scale properly
		float width = gameConfig->at("Window").at("width").as<int>();
		float height = gameConfig->at("Window").at("height").as<int>();

		// Apply the scaled factor to each vertex
		for (int i = 0; i < cpuModel.vertexCount; i++) {
			cpuModel.vertices[i].pos.x *= scale;
			cpuModel.vertices[i].pos.y *= scale * width / height;  // we must multiply here to ensure scaling is correct
		}
	}


	//translate's a model
	void translate(GW::MATH::GVECTORF translate) {

		for (int i = 0; i < cpuModel.vertices.size(); ++i) {
			cpuModel.vertices[i].pos.x += translate.x;
			cpuModel.vertices[i].pos.y += translate.y;
			cpuModel.vertices[i].pos.z += translate.z;

		}
	}

	//rotates around the y
	void rotateYAxis(float degrees) {
		float cosTheta = cos(toRad(degrees));
		float sinTheta = sin(toRad(degrees));
		for (int i = 0; i < cpuModel.vertices.size(); ++i) {
			float x = cpuModel.vertices[i].pos.x;
			float z = cpuModel.vertices[i].pos.z;
			cpuModel.vertices[i].pos.x = x * cosTheta - z * sinTheta;
			cpuModel.vertices[i].pos.z = x * sinTheta + z * cosTheta;
		}
	}

	//rotates around the x
	void rotateXAxis(float degrees) {
		float cosTheta = cos(toRad(degrees));
		float sinTheta = sin(toRad(degrees));
		for (int i = 0; i < cpuModel.vertices.size(); ++i) {
			float y = cpuModel.vertices[i].pos.y;
			float z = cpuModel.vertices[i].pos.z;
			cpuModel.vertices[i].pos.y = y * cosTheta - z * sinTheta;
			cpuModel.vertices[i].pos.z = y * sinTheta + z * cosTheta;
		}
	}
	
};

//button is a uiModel but has the ability to handle events.
class userButton : public uiModel {

public:
	float xPos; //xpos
	float yPos;	//ypos
	float width;	//button width
	float height;	//button height

	//default constructor - don't use
	userButton() {
		render = false;
	}

	//creates a button and pulls the button coordinates and size from defaults.ini
	userButton(const std::string& buttonName, GameConfig *gameConfig) {


		xPos = gameConfig->at(buttonName).at("xPos").as<float>();
		yPos = gameConfig->at(buttonName).at("yPos").as<float>();
		width = gameConfig->at(buttonName).at("width").as<int>();
		height = gameConfig->at(buttonName).at("height").as<int>();

		render = false;

	}

	void DrawModel(GameConfig *gameConfig) {

		//pull button defaults from the default ini file
		float screenWidth = gameConfig->at("Window").at("width").as<int>();
		float screenHeight = gameConfig->at("Window").at("height").as<int>();

		// Transform NDC coordinates to screen coordinates
		float screenX = (xPos + 1) / 2 * screenWidth;
		float screenY = (yPos + 1) / 2 * screenHeight;
		float screenW = width * screenWidth / 2;
		float screenH = height * screenHeight / 2;

		glBegin(GL_QUADS);
		glVertex2f(screenX, screenY);
		glVertex2f(screenX + screenW, screenY);
		glVertex2f(screenX + screenW, screenY + screenH);
		glVertex2f(screenX, screenY + screenH);
		glEnd();
	}

	//toggles a button on and off
	void toggleRender() {
		render = !render;
	}


	//Button input for any button, use the Gateware Inputs for keypress
	//gInput should be the input Proxy from render
	//onPress is a lambda funtion you want called when pressed
	void HandleInput(int keyPress, GW::INPUT::GInput gInput, std::function<void()> onPress) {

		float mouseX, mouseY;

		GW::GReturn mousePos = gInput.GetMousePosition(mouseX, mouseY);


#ifndef NDEBUG

		std::cout << "mouseX:" << mouseX << std::endl;
		std::cout << "mouseY:" << mouseY << std::endl;

#endif

		if (render)
		{
			// Check if mouse position is within button bounds
			if (mouseX >= xPos && mouseX <= xPos + width &&
				mouseY >= yPos && mouseY <= yPos + height) {

				float state;
				gInput.GetState(keyPress, state);

				// Check mouse button state (e.g., left button clicked)
				if (state > 0) {

					onPress();
				}
			}
		}

	}

	//overload for lambdas that require a Model as input
	void HandleInput(uiModel* model, int keyPress, GW::INPUT::GInput gInput, std::function<void(uiModel*)> onPress) {

		float mouseX, mouseY;

		GW::GReturn mousePos = gInput.GetMousePosition(mouseX, mouseY);

		#ifndef NDEBUG

			std::cout << "mouseX:" << mouseX << std::endl;
			std::cout << "mouseY:" << mouseY << std::endl;

		#endif

		if (render)
		{
			// Check if mouse position is within button bounds
			if (mouseX >= xPos && mouseX <= xPos + width &&
				mouseY >= yPos && mouseY <= yPos + height) {

				float state;
				gInput.GetState(keyPress, state);

				// Check mouse button state (e.g., left button clicked)
				if (state > 0) {

					onPress(model);
				}
			}
		}

	}


};

// uiPanel is based on the level_loader, and is used to load an individual panel created in blender and render seperately from other objects.
class uiPanel
{

protected:	
	std::vector<uiModel> allUiObjects;
	GameConfig* gameConfig;				//pointer that will reference the gameConfig loaded in application
	

public:
	bool render;

	uiPanel() {
		render = false;
	}

	uiPanel(GameConfig* _gameConfig) {

		gameConfig = _gameConfig;
		render = false;

	}

	/*
	// Scales a model's vertices
	void scaleObject(uiModel& object, float scale) {

		//Retrived height and width of the window to scale properly
		float width = gameConfig->at("Window").at("width").as<int>();
		float height = gameConfig->at("Window").at("height").as<int>();


		// Apply the scaled factor to each vertex
		for (int i = 0; i < object.cpuModel.vertexCount; i++) {
			object.cpuModel.vertices[i].pos.x *= scale;
			object.cpuModel.vertices[i].pos.y *= scale * width / height;  // we must multiply here to ensure scaling is correct
			object.cpuModel.vertices[i].pos.z *= scale;
		}
	}


	//translate's a model
	void translateObject(uiModel& object, GW::MATH::GVECTORF translate) {

		for (int i = 0; i < object.cpuModel.vertices.size(); ++i) {
			object.cpuModel.vertices[i].pos.x += translate.x;
			object.cpuModel.vertices[i].pos.y += translate.y;
			object.cpuModel.vertices[i].pos.z += translate.z;

		}
	}

	//rotates around the y
	void rotateObjectYAxis(uiModel& object, float degrees) {
		float cosTheta = cos(toRad(degrees));
		float sinTheta = sin(toRad(degrees));
		for (int i = 0; i < object.cpuModel.vertices.size(); ++i) {
			float x = object.cpuModel.vertices[i].pos.x;
			float z = object.cpuModel.vertices[i].pos.z;
			object.cpuModel.vertices[i].pos.x = x * cosTheta - z * sinTheta;
			object.cpuModel.vertices[i].pos.z = x * sinTheta + z * cosTheta;
		}
	}

	//rotates around the x
	void rotateObjectXAxis(uiModel& object, float degrees) {
		float cosTheta = cos(toRad(degrees));
		float sinTheta = sin(toRad(degrees));
		for (int i = 0; i < object.cpuModel.vertices.size(); ++i) {
			float y = object.cpuModel.vertices[i].pos.y;
			float z = object.cpuModel.vertices[i].pos.z;
			object.cpuModel.vertices[i].pos.y = y * cosTheta - z * sinTheta;
			object.cpuModel.vertices[i].pos.z = y * sinTheta + z * cosTheta;
		}
	}*/

	// Draws all objects in the level
	void Render() {

		// iterate over each model and tell it to draw itself
		if (render)
		{
			for (auto& e : allUiObjects) {
			
				if (e.render)
				{
					e.DrawModel();

				}

			}
		}


	}

	bool LoadMeshes(const char* gameLevelPath, const char* h2bFolderPath, GW::SYSTEM::GLog log)
	{

		log.Create("../UILevelLoaderLog.txt");
		log.LogCategorized("EVENT", "LOADING GAME LEVEL [OBJECT ORIENTED]");
		log.LogCategorized("MESSAGE", "Begin Reading Game Level Text File.");

		UnloadLevel();// clear previous level data if there is any
		GW::SYSTEM::GFile file;
		file.Create();
		if (-file.OpenTextRead(gameLevelPath)) {
			log.LogCategorized(
				"ERROR", (std::string("Game level not found: ") + gameLevelPath).c_str());
			return false;
		}
		char linebuffer[1024];
		while (+file.ReadLine(linebuffer, 1024, '\n')) {
			// having to have this is a bug, need to have Read/ReadLine return failure at EOF
			if (linebuffer[0] == '\0')
				break;
			if (std::strcmp(linebuffer, "MESH") == 0) {
				userButton* newButton = new userButton();
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
				// create the model file name from this (strip the .001)
				newButton->SetName(linebuffer);
				std::string modelFile = linebuffer;
				modelFile = modelFile.substr(0, modelFile.find_last_of("."));
				modelFile += ".h2b";

				if (strstr(linebuffer, "Button") != NULL) {

					// now read the transform data as we will need that regardless
					GW::MATH::GMATRIXF transform;
					for (int i = 0; i < 4; ++i) {
						file.ReadLine(linebuffer, 1024, '\n');
						// read floats
						std::sscanf(linebuffer + 13, "%f, %f, %f, %f",
							&transform.data[0 + i * 4], &transform.data[1 + i * 4],
							&transform.data[2 + i * 4], &transform.data[3 + i * 4]);
					}
					std::string loc = "Location: X ";
					loc += std::to_string(transform.row4.x) + " Y " +
						std::to_string(transform.row4.y) + " Z " + std::to_string(transform.row4.z);
					log.LogCategorized("INFO", loc.c_str());

					// Add new model to list of all Models
					log.LogCategorized("MESSAGE", "Begin Importing .H2B File Data.");
					modelFile = std::string(h2bFolderPath) + "/" + modelFile;
					newButton->SetWorldMatrix(transform);
					// If we find and load it add it to the level
					if (newButton->LoadModelDataFromDisk(modelFile.c_str())) {
						// add to our level objects, we use std::move since Model::cpuModel is not copy safe.
						newButton->gameConfig = gameConfig; //gives access to the gameConfig default values
						allUiObjects.push_back(std::move(*newButton));
						log.LogCategorized("INFO", (std::string("H2B Imported: ") + modelFile).c_str());
					}
					else {
						// notify user that a model file is missing but continue loading
						log.LogCategorized("ERROR",
							(std::string("H2B Not Found: ") + modelFile).c_str());
						log.LogCategorized("WARNING", "Loading will continue but model(s) are missing.");
					}
					log.LogCategorized("MESSAGE", "Importing of .H2B File Data Complete.");

					
				}

				else {
					uiModel *newModel = newButton;
					// now read the transform data as we will need that regardless
					GW::MATH::GMATRIXF transform;
					for (int i = 0; i < 4; ++i) {
						file.ReadLine(linebuffer, 1024, '\n');
						// read floats
						std::sscanf(linebuffer + 13, "%f, %f, %f, %f",
							&transform.data[0 + i * 4], &transform.data[1 + i * 4],
							&transform.data[2 + i * 4], &transform.data[3 + i * 4]);
					}
					std::string loc = "Location: X ";
					loc += std::to_string(transform.row4.x) + " Y " +
						std::to_string(transform.row4.y) + " Z " + std::to_string(transform.row4.z);
					log.LogCategorized("INFO", loc.c_str());

					// Add new model to list of all Models
					log.LogCategorized("MESSAGE", "Begin Importing .H2B File Data.");
					modelFile = std::string(h2bFolderPath) + "/" + modelFile;
					newModel->SetWorldMatrix(transform);
					// If we find and load it add it to the level
					if (newModel->LoadModelDataFromDisk(modelFile.c_str())) {
						// add to our level objects, we use std::move since Model::cpuModel is not copy safe.
						newModel->gameConfig = gameConfig;
						allUiObjects.push_back(std::move(*newModel));
						log.LogCategorized("INFO", (std::string("H2B Imported: ") + modelFile).c_str());
					}
					else {
						// notify user that a model file is missing but continue loading
						log.LogCategorized("ERROR",
							(std::string("H2B Not Found: ") + modelFile).c_str());
						log.LogCategorized("WARNING", "Loading will continue but model(s) are missing.");
					}
					log.LogCategorized("MESSAGE", "Importing of .H2B File Data Complete.");


				}

			
			}


		}

		log.LogCategorized("MESSAGE", "Game Level File Reading Complete.");
		// level loaded into CPU ram
		log.LogCategorized("EVENT", "GAME LEVEL WAS LOADED TO CPU [OBJECT ORIENTED]");

		

		return true;
	}

	// Upload the CPU level to GPU
	void UploadLevelToGPU() {
		// iterate over each model and tell it to draw itself
		for (auto& e : allUiObjects) {
			e.UploadModelData2GPU();
		}

	}

	// used to wipe CPU & GPU level data between levels
	void UnloadLevel(){
		allUiObjects.clear();
	}

	//toggles a UI panel on and off
	void toggleRender() {
		render = !render;
	}

	//place holders
	virtual void assign() {}
	virtual void arrange() {}
	virtual void start() {}
};

//uiPanel but will house the individual components of the playerHUD
class playerUi : public uiPanel {

public:
	uiModel *heart1, *heart2, *heart3, *heart4, *heart5,
			*scoreDigit1, *scoreDigit2, *scoreDigit3, *scoreDigit4,
			*levelText, *levelNum;

	userButton *button;

	playerUi() {
		render = false;
	
	}

	playerUi(GameConfig& _gameConfig) {

		gameConfig = &_gameConfig;
		render = false;

		button = new userButton("Button1", gameConfig);

	}

	void assign() override{

		heart1 = &allUiObjects[0];
		heart2 = &allUiObjects[1];
		heart3 = &allUiObjects[2];


	}

	//updates the vertices for the player HUD to be in their correct positions
	void arrange() override{

		heart1->scale(gameConfig->at("Heart1").at("scale").as<float>());
		heart1->translate({ gameConfig->at("Heart1").at("xPos").as<float>(), gameConfig->at("Heart1").at("yPos").as<float>() });

		heart2->scale(gameConfig->at("Heart2").at("scale").as<float>());
		heart2->translate({ gameConfig->at("Heart2").at("xPos").as<float>(), gameConfig->at("Heart2").at("yPos").as<float>() });

		heart3->scale(gameConfig->at("Heart3").at("scale").as<float>());
		heart3->translate({ gameConfig->at("Heart3").at("xPos").as<float>(), gameConfig->at("Heart3").at("yPos").as<float>() });

	}

	//turns default player HUD options on
	void start() override{

		heart1->toggleRender();
		heart2->toggleRender();
		heart3->toggleRender();

		button->toggleRender();

	}
};
	
//LAMBDA FUNCTIONS
//Place all Ui Related button calls here for now

//Stops Rendering specific Model 
auto turnOffRender = [](uiModel* model) {
	model->render = false;
	};

