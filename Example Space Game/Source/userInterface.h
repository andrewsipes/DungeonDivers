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

	virtual bool DrawModel(){

		//Get Block Index, and Bind the Buffer
		int blockIndex = (glGetUniformBlockIndex(shaderExecutable, "UboData"));
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOBufferObject);
		glUniformBlockBinding(shaderExecutable, blockIndex, 0);

		//update vertex buffer
		updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));

		//sets hud in front of everything else
		glDepthRange(0.1, 0.2);

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
	virtual void scale(float scale) {

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

//same as uiModel but the depth buffer has been updated
class buttonText : public uiModel
{
public:

	buttonText() {
		render = true;
		
	}

	bool DrawModel() override{

		//Get Block Index, and Bind the Buffer
		int blockIndex = (glGetUniformBlockIndex(shaderExecutable, "UboData"));
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOBufferObject);
		glUniformBlockBinding(shaderExecutable, blockIndex, 0);
		
		//update vertex buffer
		updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));

		//sets hud in front of everything else
		glDepthRange(0.0, 0.1);

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
};

//button is a uiModel but has the ability to handle events.
class userButton : public uiModel {

public:
	float xPos, yPos, width, height; //button location and dimensions

	buttonText *text;

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

	
	void scale(float scale) override{

		//Retrived height and width of the window to scale properly
		float screenWidth = gameConfig->at("Window").at("width").as<int>();
		float screenHeight = gameConfig->at("Window").at("height").as<int>();

		// Apply the scaled factor to each vertex
		for (int i = 0; i < cpuModel.vertexCount; i++) {
			cpuModel.vertices[i].pos.x *= scale;
			cpuModel.vertices[i].pos.y *= scale * screenWidth / screenHeight;  // we must multiply here to ensure scaling is correct
		}

		xPos = cpuModel.vertices[0].pos.x;
		yPos = cpuModel.vertices[0].pos.y;
		width = cpuModel.vertices[1].pos.x - cpuModel.vertices[0].pos.x;
		height = cpuModel.vertices[2].pos.y - cpuModel.vertices[0].pos.y;
	}

	/*
	//load button defaults
	void loadDefaults(std::string buttonName) {

		//get the default position
		xPos = gameConfig->at(buttonName).at("xPos").as<float>();
		yPos = gameConfig->at(buttonName).at("yPos").as<float>();

		//move the object to its intended position and apply scale
		translate({ gameConfig->at(buttonName).at("xPos").as<float>(), gameConfig->at(buttonName).at("yPos").as<float>() });
		scale(gameConfig->at(buttonName).at("scale").as<float>());

	} */

	void scale(float scaleX, float scaleY) {

		//Retrived height and width of the window to scale properly
		float screenWidth = gameConfig->at("Window").at("width").as<int>();
		float screenHeight = gameConfig->at("Window").at("height").as<int>();

		// Apply the scaled factor to each vertex
		for (int i = 0; i < cpuModel.vertexCount; i++) {
			cpuModel.vertices[i].pos.x *= scaleX;
			cpuModel.vertices[i].pos.y *= scaleY * screenWidth / screenHeight;  // we must multiply here to ensure scaling is correct
		}

		xPos = cpuModel.vertices[0].pos.x;
		yPos = cpuModel.vertices[0].pos.y;
		width = cpuModel.vertices[1].pos.x - cpuModel.vertices[0].pos.x;
		height = cpuModel.vertices[2].pos.y - cpuModel.vertices[0].pos.y;
	}
	//load button defaults
	void loadDefaults(std::string _buttonName, buttonText &_text) {

		//move the object to its intended position and apply scale
		scale(gameConfig->at(_buttonName).at("scaleX").as<float>(), gameConfig->at(_buttonName).at("scaleY").as<float>());
		translate({ gameConfig->at(_buttonName).at("xPos").as<float>(), gameConfig->at(_buttonName).at("yPos").as<float>()});

		//update Text, scale and translate it
		text = &_text;
		loadTextDefaults(_buttonName, *text);

	}

	//loads button text defaults
	void loadTextDefaults(std::string _buttonName, buttonText &_text) {
	
		//update Text size
		_text.scale(gameConfig->at(_buttonName).at("textScale").as<float>());

		//use vertice information from the button to update text location (ADJ varibles allow for manual adjustments in defaults.ini
		_text.translate({ -cpuModel.vertices[0].pos.x - gameConfig->at(_buttonName).at("xAdj").as<float>(), 
			cpuModel.vertices[1].pos.y  + gameConfig->at(_buttonName).at("yAdj").as<float>() });

		//flip the orientation
		_text.rotateYAxis(180.0f);
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
		float screenWidth = gameConfig->at("Window").at("width").as<int>();
		float screenHeight = gameConfig->at("Window").at("height").as<int>();

		GW::GReturn mousePos = gInput.GetMousePosition(mouseX, mouseY);

		mouseX = 2.0f * mouseX / screenWidth - 1.0f;
		mouseY = 1.0f - 2.0f * mouseY / screenHeight;


#ifndef NDEBUG

		//std::cout << "mouseX:" << mouseX << std::endl;
		//std::cout << "mouseY:" << mouseY << std::endl;
		//std::cout << "xPos:" << xPos << std::endl;
		//std::cout << "yPos:" << yPos << std::endl;

#endif

		if (render)
		{
			//check if mouse position is within button bounds
			if (mouseX >= cpuModel.vertices[0].pos.x && mouseX <= cpuModel.vertices[0].pos.x + width &&
				mouseY >= cpuModel.vertices[0].pos.y && mouseY <= cpuModel.vertices[0].pos.y + height) {

				float state;
				gInput.GetState(keyPress, state);

				//check if clicked
				if (state > 0) {

					onPress();
				}
			}
		}

	}

	//overload for lambdas that require a Model as input
	void HandleInput(uiModel* model, int keyPress, GW::INPUT::GInput gInput, std::function<void(uiModel*)> onPress) {

		float mouseX, mouseY;
		float screenWidth = gameConfig->at("Window").at("width").as<int>();
		float screenHeight = gameConfig->at("Window").at("height").as<int>();

		GW::GReturn mousePos = gInput.GetMousePosition(mouseX, mouseY);

		mouseX = 2.0f * mouseX / screenWidth - 1.0f;
		mouseY = 1.0f - 2.0f * mouseY / screenHeight;


		#ifndef NDEBUG

			std::cout << "mouseX:" << mouseX << std::endl;
			std::cout << "mouseY:" << mouseY << std::endl;
			std::cout << "xPos:" << xPos << std::endl;
			std::cout << "yPos:" << yPos << std::endl;
		

		#endif

		

		if (render)
		{
			//check if mouse position is within button bounds
			if (mouseX >= cpuModel.vertices[0].pos.x && mouseX <= cpuModel.vertices[0].pos.x + width &&
				mouseY >= cpuModel.vertices[0].pos.y && mouseY <= cpuModel.vertices[0].pos.y + height) {

				float state;

				gInput.GetState(keyPress, state);

				//check if clicked
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
	std::vector<userButton> allUiButtonObjects;
	std::vector<buttonText> allUiButtonTextObjects;

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


	// Draws all objects in the level
	void Render() {

		// iterate over each model and tell it to draw itself
		if (render)
		{
			for (auto& e : allUiObjects) {
			
				if (e.render)
					e.DrawModel();

			}

			for (auto& f : allUiButtonObjects){

				if (f.render) {
					f.DrawModel();
					f.text->DrawModel();
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

				file.ReadLine(linebuffer, 1024, '\n');

				//CHECK FOR BUTTON TEXT
				if (strstr(linebuffer, "ButtonText") != NULL) {
			
					buttonText* newButtonText = new buttonText();
					log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
					// create the model file name from this (strip the .001)
					newButtonText->SetName(linebuffer);
					std::string modelFile = linebuffer;
					modelFile = modelFile.substr(0, modelFile.find_last_of("."));
					modelFile += ".h2b";

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
					newButtonText->SetWorldMatrix(transform);
					// If we find and load it add it to the level
					if (newButtonText->LoadModelDataFromDisk(modelFile.c_str())) {
						// add to our level objects, we use std::move since Model::cpuModel is not copy safe.
						newButtonText->gameConfig = gameConfig;
						allUiButtonTextObjects.push_back(std::move(*newButtonText));
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

				//CHECK FOR BUTTONS
				else if (strstr(linebuffer, "Button") != NULL) {

					userButton* newButton = new userButton();
					log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
					// create the model file name from this (strip the .001)
					newButton->SetName(linebuffer);
					std::string modelFile = linebuffer;
					modelFile = modelFile.substr(0, modelFile.find_last_of("."));
					modelFile += ".h2b";

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
						allUiButtonObjects.push_back(std::move(*newButton));
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

				//EVERYTHING ELSE
				else {

					uiModel* newModel = new uiModel();
					log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
					// create the model file name from this (strip the .001)
					newModel->SetName(linebuffer);
					std::string modelFile = linebuffer;
					modelFile = modelFile.substr(0, modelFile.find_last_of("."));
					modelFile += ".h2b";

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

		for (auto& f : allUiButtonObjects) {
			f.UploadModelData2GPU();
			f.text->UploadModelData2GPU();
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

	}

	void assign() override{

		heart1 = &allUiObjects[0];
		button = &allUiButtonObjects[0];

		//heart2 = &allUiObjects[1];
		//heart3 = &allUiObjects[2];


	}

	//updates the vertices for the player HUD to be in their correct positions
	void arrange() override{

		heart1->scale(gameConfig->at("Heart1").at("scale").as<float>());
		heart1->translate({ gameConfig->at("Heart1").at("xPos").as<float>(), gameConfig->at("Heart1").at("yPos").as<float>() });

		//heart2->scale(gameConfig->at("Heart2").at("scale").as<float>());
		//heart2->translate({ gameConfig->at("Heart2").at("xPos").as<float>(), gameConfig->at("Heart2").at("yPos").as<float>() });

		//heart3->scale(gameConfig->at("Heart3").at("scale").as<float>());
		//heart3->translate({ gameConfig->at("Heart3").at("xPos").as<float>(), gameConfig->at("Heart3").at("yPos").as<float>() });

	

		button->loadDefaults("Button1", allUiButtonTextObjects[0]);
		

	}

	//turns default player HUD options on
	void start() override{

		heart1->toggleRender();
		button->toggleRender();

		//heart2->toggleRender();
		//heart3->toggleRender();


	}
};
	
//LAMBDA FUNCTIONS
//Place all Ui Related button calls here for now

//Stops Rendering specific Model 
auto turnOffRender = [](uiModel* model) {
	model->render = false;
	};

