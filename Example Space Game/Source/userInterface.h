#include "./load_object_oriented.h"

class uiModel : public Model
{
	bool render;

public:
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


};

class uiPanel : public Level_Objects
{
public:
	bool render;
	std::vector<uiModel> allUiObjectsInLevel;

	uiPanel() {

		render = false;

	}

	//scales a model's vertices
	void scaleObject(uiModel& object, float scale) {

		for (int i = 0; i < object.cpuModel.vertices.size(); ++i) {
			object.cpuModel.vertices[i].pos.x *= scale;
			object.cpuModel.vertices[i].pos.z *= scale;
			object.cpuModel.vertices[i].pos.y *= scale;

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
	void Render(GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection) override{

		// iterate over each model and tell it to draw itself
		for (auto& e : allUiObjectsInLevel) {
			e.DrawModel(_ogl, _camera, _view, _projection, sunLight, LIGHTDATA);
		}


	}

	bool LoadMeshes(const char* gameLevelPath,
		const char* h2bFolderPath,
		GW::SYSTEM::GLog log, GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection) override
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
					allUiObjectsInLevel.push_back(std::move(newModel));
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
	void UploadLevelToGPU(GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection) override{
		// iterate over each model and tell it to draw itself
		for (auto& e : allUiObjectsInLevel) {
			e.UploadModelData2GPU(_ogl, _camera, _view, _projection, sunLight, LIGHTDATA);
		}

	}
};
	


