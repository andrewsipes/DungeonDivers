#include "./load_object_oriented.h"

class uiModel : public Model
{

public:
	bool render;

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

};

//Base UI panel class that works similar to Level_Objects
//UI elements are read from a text file and they render themselves.
class uiPanel
{

protected:	
	std::vector<uiModel> allUiObjects;

public:
	bool render;

	uiPanel() {

		render = false;

	}

	//scales a model's vertices
	void scaleObject(uiModel& object, double scale) {

		for (int i = 0; i < object.cpuModel.vertexCount; i++) {
			object.cpuModel.vertices[i].pos.x = (double)object.cpuModel.vertices[i].pos.x * scale;
			object.cpuModel.vertices[i].pos.y = (double)object.cpuModel.vertices[i].pos.y * scale;
			object.cpuModel.vertices[i].pos.z = (double)object.cpuModel.vertices[i].pos.z * scale;

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
	}

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
				uiModel newModel;
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
				// create the model file name from this (strip the .001)
				newModel.SetName(linebuffer);
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
				newModel.SetWorldMatrix(transform);
				// If we find and load it add it to the level
				if (newModel.LoadModelDataFromDisk(modelFile.c_str())) {
					// add to our level objects, we use std::move since Model::cpuModel is not copy safe.
					allUiObjects.push_back(std::move(newModel));
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
class playerHUD : public uiPanel {

public:

//lives
	uiModel* heart1;
	uiModel* heart2;
	uiModel* heart3;
	uiModel* heart4;
	uiModel* heart5;

//score
	uiModel* scoreDigit1;
	uiModel* scoreDigit2;
	uiModel* scoreDigit3;
	uiModel* scoreDigit4;

//level text
	uiModel* levelText;
	uiModel* levelNum;

	void assign() override{

		heart1 = &allUiObjects[0];
		heart2 = &allUiObjects[1];
		heart3 = &allUiObjects[2];

	}

	//updates the vertices for the player HUD to be in their correct positions
	void arrange() override{

		scaleObject(*heart1, .07f);
		translateObject(*heart1, { -.9, .9,0 });

		scaleObject(*heart2, .07f);
		translateObject(*heart2, { -.75, .9,0 });

		scaleObject(*heart3, .07f);
		translateObject(*heart3, { -.6, .9,0 });

	}

	//turns default player HUD options on
	void start() override{


		heart1->toggleRender();
		heart2->toggleRender();
		heart3->toggleRender();



	}
};
	


