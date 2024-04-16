#include "./load_object_oriented.h"

//This entire .H file handles the UserInterface Classes, and their accompanying methods
//In order to use this properly, objects should be created in render.h and handled as needed for rendering, events, etc.

//uiModel is a derivate of the original model with updated definitions specific to UI
class uiModel : public Model
{

public:
	bool render;
	float alpha = 1.0f;

	GameConfig* gameConfig;

	uiModel(){
		render = false;
	}

	void SetUpPipeline(float alpha) {
		glUseProgram(shaderExecutable);
		glBindVertexArray(vertexArray);
		SetVertexAttributes();

		glUniform1i(glGetUniformLocation(shaderExecutable, "isUi"), true);
		glUniform1f(glGetUniformLocation(shaderExecutable, "alpha"), alpha);
	}

	void updateUniformBufferObject(const H2B::MATERIAL _material, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj) {
		glBindBuffer(GL_UNIFORM_BUFFER, UBOBufferObject);
		ubo = updateUboInstance(_material, _camera, _view, _proj);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ubo), &ubo);
	}

	//assigns ubo data to send to the shader
	UBO_DATA updateUboInstance(H2B::MATERIAL _material, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj){

		UBO_DATA _ubo;
		
		_ubo._view = _view;
		_ubo._proj= _proj;
		_ubo._cam = _camera;
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

	virtual bool DrawModel(GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj, float alpha){

		//Get Block Index, and Bind the Buffer
		int blockIndex = (glGetUniformBlockIndex(shaderExecutable, "UboData"));
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOBufferObject);
		glUniformBlockBinding(shaderExecutable, blockIndex, 0);

		//update vertex buffer
		updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));

		//sets hud in front of everything else
		glDepthRange(uiModelDepth, uiOverlayDepth);

		//Draw meshes - iterates through the meshes and materials to draw them individually.
		for (int j = 0; j < cpuModel.meshCount; j++) {
			updateUniformBufferObject(cpuModel.materials[cpuModel.meshes[j].materialIndex], _camera, _view, _proj);
			SetUpPipeline(alpha);
			updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));
			glDrawElements(GL_TRIANGLES, cpuModel.meshes[j].drawInfo.indexCount, GL_UNSIGNED_INT, (void*)(cpuModel.meshes[j].drawInfo.indexOffset * sizeof(cpuModel.indices[0])));
		}

		glBindVertexArray(0);
		glUseProgram(0);

		return true;

	}

	bool UploadModelData2GPU(GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj) {	
		ubo = updateUboInstance(cpuModel.materials[0], _camera,  _view,  _proj);

		InitializeGraphics();


		return true;

	}

	void toggleRender() {
		render = !render;
	}

	//Uses the world matrix and adjusts it for placing each UI object properly
	virtual void loadDefaults(){
		scale(-world.row1.x);
		translate({ -world.row4.x, world.row4.y});
		rotateYAxis(180.0f);

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

//updated uiModel class for drawing a different depth buffer
class uiOverlay :public uiModel {

public:
	bool DrawModel(GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj, float alpha) override{

		//Get Block Index, and Bind the Buffer
		int blockIndex = (glGetUniformBlockIndex(shaderExecutable, "UboData"));
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOBufferObject);
		glUniformBlockBinding(shaderExecutable, blockIndex, 0);

		//update vertex buffer
		updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));

		//sets hud in front of everything else
		glDepthRange(uiOverlayDepth, worldDepth);

		//Draw meshes - iterates through the meshes and materials to draw them individually.
		for (int j = 0; j < cpuModel.meshCount; j++) {
			updateUniformBufferObject(cpuModel.materials[cpuModel.meshes[j].materialIndex], _camera, _view, _proj);
			SetUpPipeline(alpha);
			updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));
			glDrawElements(GL_TRIANGLES, cpuModel.meshes[j].drawInfo.indexCount, GL_UNSIGNED_INT, (void*)(cpuModel.meshes[j].drawInfo.indexOffset * sizeof(cpuModel.indices[0])));
		}

		glBindVertexArray(0);
		glUseProgram(0);

		return true;

	}


	void scale(float scaleX, float scaleY) {

		//Retrived height and width of the window to scale properly
		float screenWidth = gameConfig->at("Window").at("width").as<int>();
		float screenHeight = gameConfig->at("Window").at("height").as<int>();

		// Apply the scaled factor to each vertex
		for (int i = 0; i < cpuModel.vertexCount; i++) {
			cpuModel.vertices[i].pos.x *= scaleX;
			cpuModel.vertices[i].pos.y *= scaleY * screenWidth / screenHeight;  // we must multiply here to ensure scaling is correct
		}

	}

	//Uses the world matrix and adjusts it for placing each UI object properly
	void loadDefaults() override {

		//load alpha if it exists
		try {
			alpha = gameConfig->at(this->name).at("alpha").as<float>();
		}
		catch (const std::out_of_range& e) {

			std::cerr << "WARNING: NO ALPHA FOUND" << std::endl;
		}

		scale(-world.row1.x, world.row2.y);
		translate({ world.row4.x, world.row4.y });

	

	}

};

//same as uiModel but the depth buffer has been updated
class buttonText : public uiModel
{
public:

	buttonText() {
		render = true;
		
	}

	bool DrawModel(GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj, float alpha) override{

		//Get Block Index, and Bind the Buffer
		int blockIndex = (glGetUniformBlockIndex(shaderExecutable, "UboData"));
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOBufferObject);
		glUniformBlockBinding(shaderExecutable, blockIndex, 0);
		
		//update vertex buffer
		updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));

		//sets hud in front of everything else
		glDepthRange(userButtonTextDepth, userButtonDepth);

		//Draw meshes - iterates through the meshes and materials to draw them individually.
		for (int j = 0; j < cpuModel.meshCount; j++) {
			updateUniformBufferObject(cpuModel.materials[cpuModel.meshes[j].materialIndex], _camera, _view, _proj);
			SetUpPipeline(alpha);
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


	void scale(float scaleX, float scaleY) {

		//Retrived height and width of the window to scale properly
		float screenWidth = gameConfig->at("Window").at("width").as<int>();
		float screenHeight = gameConfig->at("Window").at("height").as<int>();

		// Apply the scaled factor to each vertex
		for (int i = 0; i < cpuModel.vertexCount; i++) {
			cpuModel.vertices[i].pos.x *= scaleX;
			cpuModel.vertices[i].pos.y *= scaleY * screenWidth / screenHeight;  // we must multiply here to ensure scaling is correct
		}

	}

	//Uses the world matrix and adjusts it for placing each UI object properly
	void loadDefaults() override{

		//load alpha if it exists
		try {
			alpha = gameConfig->at(this->name).at("alpha").as<float>();
		}
		catch (const std::out_of_range& e) {

			std::cerr << "WARNING: NO ALPHA FOUND" << std::endl;
		}

		scale(-world.row1.x, world.row2.y); 
		translate({ world.row4.x, world.row4.y });
		
		//update button variables based on new vertex info
		xPos = cpuModel.vertices[0].pos.x;
		yPos = cpuModel.vertices[0].pos.y;
		width = cpuModel.vertices[1].pos.x - cpuModel.vertices[0].pos.x;
		height = cpuModel.vertices[2].pos.y - cpuModel.vertices[0].pos.y;

		loadTextDefaults();

	}

	//loads button text defaults
	void loadTextDefaults() {	
		text->scale(-1*text->world.row1.x);
		text->translate({-1*text->world.row4.x, text->world.row4.y});
		text->rotateYAxis(180.0f);
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

		if (render)		{
			//check if mouse position is within button bounds
			if (mouseX >= xPos && mouseX <= xPos + width &&
				mouseY >= yPos && mouseY <= yPos + height) {

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

		/*	std::cout << "mouseX:" << mouseX << std::endl;
			std::cout << "mouseY:" << mouseY << std::endl;
			std::cout << "xPos:" << xPos << std::endl;
			std::cout << "yPos:" << yPos << std::endl;*/
		

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
	void Render(GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj) {

		// iterate over each model and tell it to draw itself
		if (render)
		{
			for (auto& e : allUiObjects) {		
				if (e.render)
					e.DrawModel( _camera, _view,  _proj, e.alpha);
			}

			for (auto& f : allUiButtonObjects){
				if (f.render) {
					f.DrawModel(_camera, _view, _proj, f.alpha);
					f.text->DrawModel(_camera, _view, _proj, f.text->alpha);
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
	void UploadLevelToGPU(GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj) {
		// iterate over each model and tell it to draw itself
		for (auto& e : allUiObjects) {
			e.UploadModelData2GPU(_camera, _view, _proj);
		}

		for (auto& f : allUiButtonObjects) {
			f.UploadModelData2GPU(_camera,  _view, _proj);
			f.text->UploadModelData2GPU(_camera, _view, _proj);
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
	virtual void arrange() {
	
		for (userButton &button : allUiButtonObjects)		{
			for (buttonText &text : allUiButtonTextObjects){
				if (text.name.find(button.name) != std::string::npos){
					button.loadDefaults();
				}
			}
		}
	}
	virtual void start() {}

};

//uiPanel but will house the individual components of the playerHUD
class playerUi : public uiPanel {

public:
	//uiModel* heart0, *heart1, * heart2, * heart3, * heart4, * heart5, * heart6, * heart7;
	uiModel	* levelText, *startText, *pauseText, *levelCompleteText;
	std::vector<uiModel*> hearts, levelDigit, scoreDigit1, scoreDigit2, scoreDigit3, scoreDigit4;

	userButton *button;

	playerUi() {
		render = false;
	}

	playerUi(GameConfig& _gameConfig) {
		gameConfig = &_gameConfig;
		render = false;
	}
	
	//updates Score UI
	void updateScore(int score){
		
		//prevents score from going too high
		if (score > 9999)
		{
			score = 9999;
		}

		int num = score;
		int digit;		
		int displayScore[4] = { 0,0,0,0 }; //zero until filled
		int iter = 3;

		//toggle all digits off
		for (uiModel* digit : scoreDigit1){
			digit->render = false;
		}
		for (uiModel* digit : scoreDigit2) {
			digit->render = false;
		}
		for (uiModel* digit : scoreDigit3) {
			digit->render = false;
		}
		for (uiModel* digit : scoreDigit4) {
			digit->render = false;
		}


		//Loop that takes the int that was passed in and seperates by digit
		while (num > 0) {
			digit = num % 10;				//get the digit by using mod
			displayScore[iter] = digit;		//update an array so we have the right values
			num /= 10;						//remove the last digit
			
			iter--;
		}

		//Used the extracted values to update score
		scoreDigit4[displayScore[3]]->toggleRender();
		scoreDigit3[displayScore[2]]->toggleRender();
		scoreDigit2[displayScore[1]]->toggleRender();
		scoreDigit1[displayScore[0]]->toggleRender();

	}

	//updates Hearts UI
	void updateHearts(int life) {

		//prevents life from going too high
		if (life > 8)
		{
			life = 8;
		}

		//turn all hearts off
		for (uiModel* heart : hearts)
		{
			heart->render = false;
		}

		//turn on the ones needed
		for (int i = 0; i < life; i++)
		{
			hearts[i]->toggleRender();
		}

	}

	//combines updating score and life into single method
	void update(int score, int life) {
		updateHearts(life);
		updateScore(score);
	}
	
	//assigns the panel elements to the appropiate pointers so we can control them easily
	void assign() override{

		//text
		levelText = &allUiObjects[8];
		pauseText = &allUiObjects[9];
		startText = &allUiObjects[10];
		levelCompleteText = &allUiObjects[11];

		//assign hearts
		for (int i = 0; i < 8; i++){
			hearts.push_back(&allUiObjects[0 + i]);
		}

		//assign level digit numbers
		for (int i = 0; i < 10; i++) {
			levelDigit.push_back(&allUiObjects[i + 12]);
		}

		//assign score digit numbers
		for (int i = 0; i < 10; i++) {
			scoreDigit1.push_back(&allUiObjects[i + 22]);
		}

		//assign score digit numbers
		for (int i = 0; i < 10; i++) {
			scoreDigit2.push_back(&allUiObjects[i + 32]);
		}

		//assign score digit numbers
		for (int i = 0; i < 10; i++) {
			scoreDigit3.push_back(&allUiObjects[i + 42]);
		}

		//assign score digit numbers
		for (int i = 0; i < 10; i++) {
			scoreDigit4.push_back(&allUiObjects[i + 52]);
		}

	}

	//updates the vertices for the player HUD to be in their correct positions
	void arrange() override{

		levelText->loadDefaults();
		pauseText->loadDefaults();
		startText->loadDefaults();
		levelCompleteText->loadDefaults();

		for (uiModel* heart : hearts){
			heart->loadDefaults();
		}

		for (uiModel* digit : levelDigit){
			digit->loadDefaults();
		}

		for (uiModel* digit : scoreDigit1){
			digit->loadDefaults();
		}

		for (uiModel* digit : scoreDigit2){
			digit->loadDefaults();
		}

		for (uiModel* digit : scoreDigit3){
			digit->loadDefaults();
		}

		for (uiModel* digit : scoreDigit4){
			digit->loadDefaults();
		}
	}

	//turns default playerHUD on
	void start() override{

		//Set Default values
		updateHearts(gameConfig->at("Player1").at("hearts").as<int>());
		updateScore(gameConfig->at("Player1").at("score").as<int>());
		levelText->toggleRender();
		levelDigit[0]->toggleRender();

	}
};

//uiPanel for the main menu
class mainMenuUi :	public uiPanel
{
public: 
	uiModel* gameText;
	userButton* startButton, * controlsButton, * exitButton;

	mainMenuUi() {
		render = false;
	}

	mainMenuUi(GameConfig& _gameConfig) {
		gameConfig = &_gameConfig;
		render = false;
	}

	void assign() override{
		gameText = &allUiObjects[0];
		startButton = &allUiButtonObjects[0];
		controlsButton = &allUiButtonObjects[1];
		exitButton = &allUiButtonObjects[2];
	}

	void arrange() override {

		gameText->loadDefaults();

		for (userButton& _button : allUiButtonObjects){
			for (buttonText& _text : allUiButtonTextObjects){
				if (_text.name.find(_button.name) != std::string::npos){
					_button.text = &_text;
					_button.loadDefaults();
				}
			}
		}
	}

	void start() override {
		gameText->toggleRender();
		startButton->toggleRender();
		controlsButton->toggleRender();
		exitButton->toggleRender();
	}
};

class pauseMenuUi :public uiPanel {
public:
	uiModel* pauseMenuText;
	uiOverlay* pauseOverlay;
	userButton *controlsPauseMenuButton, *resumePauseMenuButton, *restartPauseMenuButton, *exitPauseMenuButton;

	pauseMenuUi() {
		render = false;
	}

	pauseMenuUi(GameConfig& _gameConfig) {
		gameConfig = &_gameConfig;
		render = false;
	}

	void assign() override {
		
		pauseOverlay = static_cast<uiOverlay*>(&allUiObjects[0]);
		pauseMenuText = new buttonText();
		pauseMenuText = static_cast<buttonText*>(&allUiObjects[1]);
		controlsPauseMenuButton = &allUiButtonObjects[0];
		restartPauseMenuButton = &allUiButtonObjects[1];
		exitPauseMenuButton = &allUiButtonObjects[2];
		resumePauseMenuButton = &allUiButtonObjects[3];
	}

	void arrange() override {

		pauseMenuText->loadDefaults();

		for (userButton& _button : allUiButtonObjects) {
			for (buttonText& _text : allUiButtonTextObjects) {
				if (_text.name.find(_button.name) != std::string::npos) {
					_button.text = &_text;
					_button.loadDefaults();
				}
			}
		}
	}

	void start() override {
		pauseMenuText->toggleRender();
		controlsPauseMenuButton->toggleRender();
		resumePauseMenuButton->toggleRender();
		restartPauseMenuButton->toggleRender();
		exitPauseMenuButton->toggleRender();
		pauseOverlay->toggleRender();

	
	}

	// Draws all objects in the level
	void Render(GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _proj) {

		// iterate over each model and tell it to draw itself
		if (render)
		{
			for (auto& e : allUiObjects) {
				if (e.name == "pauseMenuText" || e.name == "pauseMenuOverlay"){				
					// do nothing
				}
				else if (e.render)
					e.DrawModel(_camera, _view, _proj, e.alpha);
			}

			for (auto& f : allUiButtonObjects) {
				if (f.render) {
					f.DrawModel(_camera, _view, _proj, f.alpha);
					f.text->DrawModel(_camera, _view, _proj, f.text->alpha);
				}
			}

			if (pauseOverlay->render) {
				pauseOverlay->DrawModel(_camera, _view, _proj, pauseOverlay->alpha);
			}

			if (pauseMenuText->render) {
				pauseMenuText->DrawModel(_camera, _view, _proj, pauseMenuText->alpha);
			}
		}
	}

};
	
//LAMBDA FUNCTIONS
//Place all Ui Related button calls here for now

//Stops Rendering specific Model 
auto turnOffRender = [](uiModel* model) {
	model->render = false;
	};

