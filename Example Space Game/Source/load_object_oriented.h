#include "./FileIntoString.h"
#include "./stb_image.h"
#include "./OpenGLExtensions.h"
#include "./defines.h"

//credit to stb image for their image uploader https://github.com/nothings/stb
//credit to LearnOpenGL for the skybox tutorial and skybox images //credits to learnOpenGL https://learnopengl.com/Advanced-OpenGL/Cubemaps


//converts degrees to radians
float toRad(float degrees)
{
	return (degrees * 3.14159265358979323846 / 180);
}

// Used to print debug infomation from OpenGL, pulled straight from the official OpenGL wiki.
void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}


#ifndef NDEBUG
void APIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam) {


	std::string errMessage;
	errMessage = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "");
	errMessage += " type = ";
	errMessage += type;
	errMessage += ", severity = ";
	errMessage += severity;
	errMessage += ", message = ";
	errMessage += message;
	errMessage += "\n";

	PrintLabeledDebugString("GL CALLBACK: ", errMessage.c_str());
}
#endif


// class Model contains everyhting needed to draw a single 3D model
class Model {
public:

	// Name of the Model in the GameLevel (useful for debugging)
	std::string name;

	// Loads and stores CPU model data from .h2b file
	H2B::Parser cpuModel; // reads the .h2b format

	// Shader variables needed by this model. 
	GW::MATH::GMATRIXF world;

	//cube stuff
	std::string skyBox = "";
	
	// TODO: API Rendering vars here (unique to this model)
	GLuint vertexBufferObject;
	GLuint indexBufferObject;
	GLuint lightBufferObject;
	GLuint vertexArray;
	GLuint UBOBufferObject;
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint shaderExecutable;

	inline void SetName(std::string modelName) {
		name = modelName;
	}
	inline void SetWorldMatrix(GW::MATH::GMATRIXF worldMatrix) {
		world = worldMatrix;
	}

	bool LoadModelDataFromDisk(const char* h2bPath) {
		// if this succeeds "cpuModel" should now contain all the model's info
		return cpuModel.Parse(h2bPath);
	}

	bool UploadModelData2GPU(GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection, SUNLIGHT_DATA _sLight, std::vector<LIGHT_DATA> _lights){

		// TODO: Use chosen API to upload this model's graphics data to GPU
		lbo = updateLights(_lights);
		ubo = updateUboInstance(cpuModel.materials[0], world, _camera, _view, _projection, _sLight);

		InitializeGraphics();
		
		if(name == "skyBox")
			createCubeMap(skyBox);  //credits to learnOpenGL for the skybox image https://learnopengl.com/Advanced-OpenGL/Cubemaps

		return true;

	}

	virtual bool DrawModel(GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection, SUNLIGHT_DATA _sLight, const std::vector <LIGHT_DATA>& _lights) {
		
		//keeps objects from clipping into ui
		glDepthRange(0.05, 1);

		//Get Block Index, and Bind the Buffer
		int blockIndex = (glGetUniformBlockIndex(shaderExecutable, "UboData"));
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBOBufferObject);
		glUniformBlockBinding(shaderExecutable, blockIndex, 0);

		//Get Block Index, and Bind the Buffer
		int lightBlockIndex = glGetUniformBlockIndex(shaderExecutable, "lightData");
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightBufferObject);
		glUniformBlockBinding(shaderExecutable, lightBlockIndex, 1);
		
			//updates the buffers 
			updateLightBufferObject(_lights);
			updateVertexBufferObject(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));

			//draw using elements instead for skybox
			if (name == "skyBox"){
				SetUpPipeline();
				glDrawElements(GL_TRIANGLES, 36 * 2, GL_UNSIGNED_INT, 0); // we need to double the # of indices since we are using triangles since the mesh's index count 
																		  // doesn't draw all triangles
			}

			else {

				//Draw meshes - iterates through the meshes and materials to draw them individually.
				for (int j = 0; j < cpuModel.meshCount; j++) {
					updateUniformBufferObject(cpuModel.materials[cpuModel.meshes[j].materialIndex], world, _camera, _view, _projection, _sLight);
					SetUpPipeline();
					glDrawElements(GL_TRIANGLES, cpuModel.meshes[j].drawInfo.indexCount, GL_UNSIGNED_INT, (void*)(cpuModel.meshes[j].drawInfo.indexOffset * sizeof(cpuModel.indices[0])));
				}
			}
			
		glBindVertexArray(0);
		glUseProgram(0);

	

		return true;
	}

	bool FreeResources(/*specific API device for unloading*/) {
		// TODO: Use chosen API to free all GPU resources used by this model

		glDeleteBuffers(1, &vertexBufferObject);
		glDeleteBuffers(1, &indexBufferObject);
		glDeleteBuffers(1, &UBOBufferObject);
		glDeleteBuffers(1, &lightBufferObject);
		glDeleteVertexArrays(1, &vertexArray);
		glBindVertexArray(0);
		return true;
	}

	// Sets up the buffers for all our data
	void InitializeGraphics()
	{
		// In debug mode we link openGL errors to the console
#ifndef NDEBUG
		BindDebugCallback();
#endif
		InitializeVertexBuffer();
		CreateIndexBuffer(cpuModel.indices.data(), cpuModel.indices.size() * sizeof(unsigned int));
		CreateUBOBuffer(&ubo, sizeof(ubo));
		CreateLightBuffer(&lbo, (lbo.size() * sizeof(LIGHT_DATA)));
		CompileVertexShader();
		CompileFragmentShader();
		CreateExecutableShaderProgram();

		//if we encounter the skybox model, then we need to bidn the texture for the cube.
		if (name == "skyBox")
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapTexture);
		}
	}

#ifndef NDEBUG
	void BindDebugCallback()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
	}
#endif

	void CompileVertexShader()
	{
		char errors[1024];
		GLint result;

		vertexShader = glCreateShader(GL_VERTEX_SHADER);

		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.glsl");
		const char* strings[1] = { vertexShaderSource.c_str() };
		const GLint lengths[1] = { vertexShaderSource.length() };
		glShaderSource(vertexShader, 1, strings, lengths);

		glCompileShader(vertexShader);
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);

		if (result == false)
		{
			glGetShaderInfoLog(vertexShader, 1024, NULL, errors);
			PrintLabeledDebugString("Vertex Shader Errors:\n", errors);
			abort();
			return;
		}
	}

	void CompileFragmentShader()
	{
		char errors[1024];
		GLint result;

		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		std::string fragmentShaderSource = ReadFileIntoString("../Shaders/FragmentShader.glsl");
		const char* strings[1] = { fragmentShaderSource.c_str() };
		const GLint lengths[1] = { fragmentShaderSource.length() };
		glShaderSource(fragmentShader, 1, strings, lengths);

		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);

		if (result == false)
		{
			glGetShaderInfoLog(fragmentShader, 1024, NULL, errors);
			PrintLabeledDebugString("Fragment Shader Errors:\n", errors);
			abort();
			return;
		}
	}

	void CreateExecutableShaderProgram()
	{
		char errors[1024];
		GLint result;

		shaderExecutable = glCreateProgram();
		glAttachShader(shaderExecutable, vertexShader);
		glAttachShader(shaderExecutable, fragmentShader);
		glLinkProgram(shaderExecutable);
		glGetProgramiv(shaderExecutable, GL_LINK_STATUS, &result);
		if (result == false)
		{
			glGetProgramInfoLog(shaderExecutable, 1024, NULL, errors);
			std::cout << errors << std::endl;
		}
	}

	//assigns ubo data to send to the shader
	UBO_DATA updateUboInstance(H2B::MATERIAL _material, GW::MATH::GMATRIXF _world, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection, SUNLIGHT_DATA _sLight) {

		UBO_DATA _ubo;

		_ubo.sunColor = _sLight.color;
		_ubo.sunDirection = _sLight.direction;
		_ubo.sunAmbient = _sLight.ambient;
		_ubo._view = _view;
		_ubo._proj = _projection;
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

		//here we need to send the number of lights to save some space on the lbo
		_ubo.numLights = lbo.size();

		return _ubo;
	}

	//assigns Light data to send to the buffer
	std::vector<LIGHT_DATA> updateLights(const std::vector<LIGHT_DATA>& _lights) {

		std::vector<LIGHT_DATA> _lightData = _lights;

		return _lightData;
	}

	void InitializeVertexBuffer()
	{
		CreateVertexBuffer(cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX) );
	}

	void CreateVertexBuffer(const void* data, unsigned int sizeInBytes)
	{
		glGenVertexArrays(1, &vertexArray);
		glGenBuffers(1, &vertexBufferObject);
		glBindVertexArray(vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);
	}

	void CreateIndexBuffer(const void* data, unsigned int sizeInBytes)
	{
		glGenBuffers(1, &indexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);

	}

	void CreateUBOBuffer(const void* data, unsigned int sizeInBytes)
	{
		glGenBuffers(1, &UBOBufferObject);
		glBindBuffer(GL_UNIFORM_BUFFER, UBOBufferObject);
		glBufferData(GL_UNIFORM_BUFFER, sizeInBytes, data, GL_DYNAMIC_DRAW);

	}

	void CreateLightBuffer(const void* data, unsigned int sizeInBytes)	{
		glGenBuffers(1, &lightBufferObject);
		glBindBuffer(GL_UNIFORM_BUFFER, lightBufferObject);
		glBufferData(GL_UNIFORM_BUFFER, sizeInBytes, data, GL_DYNAMIC_DRAW);

	}
	void updateLightBufferObject(const std::vector <LIGHT_DATA>& _lights) {

		glBindBuffer(GL_UNIFORM_BUFFER, lightBufferObject);
		lbo = _lights;
		glBufferSubData(GL_UNIFORM_BUFFER, 0, (lbo.size() * sizeof(LIGHT_DATA)), lbo.data());

	}

	void updateVertexBufferObject(const void* newData, GLsizei dataSize) {
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, newData);
	}

	void updateIndexBufferObject(const void* newData, GLsizei dataSize) {

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, dataSize, newData);
	}

	void updateUniformBufferObject(const H2B::MATERIAL _material, GW::MATH::GMATRIXF _world, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection, SUNLIGHT_DATA _sLight) {

		glBindBuffer(GL_UNIFORM_BUFFER, UBOBufferObject);
		ubo = updateUboInstance(_material, _world, _camera, _view, _projection, _sLight);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ubo), &ubo);
	}

	virtual void SetUpPipeline() {

		glUseProgram(shaderExecutable);
		glBindVertexArray(vertexArray);
		SetVertexAttributes();


		if (name == "skyBox")
		{	
			bool isSkybox = true;
			glUniform1i(glGetUniformLocation(shaderExecutable, "isSkybox"), isSkybox);
			glUniform1i(glGetUniformLocation(shaderExecutable, "skybox"), 0);
		}


	}

	void SetVertexAttributes()
	{
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
	}

	// creats the texture id, and assigns the faces the texture in a loop
	void createCubeMap(std::string filepath) {
		
		unsigned int texture;
		int width, height, channels;
		unsigned char* data;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

		for (int i = 0; i < 6; i++)
		{
			//load the image
			data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
			if (!data)
			{
				//output an error and continue running if no data found.
				std::cout << "[ERROR]: CUBE TEXTURE NOT FOUND" << std::endl;

				return;
			}
			//assign the texture to the face
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			//free the data
			stbi_image_free(data);
		}
		
		//required texture parameters
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		CubeMapTexture = texture;
	}


	//converts degrees to radians
	float toRad(float degrees)
	{
		return (degrees * 3.14159265358979323846 / 180);
	}
};


// class Level_Objects is simply a list of all the Models currently used by the level
class Level_Objects {


private:
	// store all our models
	std::vector<Model> allObjectsInLevel;

	//sunLight stuff
	SUNLIGHT_DATA sunLight;
	GW::MATH::GVECTORF sunLightDir;
	GW::MATH::GVECTORF sunLightColor;
	GW::MATH::GVECTORF sunLightAmbient;
	GW::MATH::GVECTORF cameraForward;

	//light Vectors, 
	std::vector<LIGHT_DATA> LIGHTDATA;	//this vector uses the structure for lighting in the lbo, we use this to hold the necessary data until moved
	std::vector<Light> lights;			//this vector will show all the data pulled from the textfile

public:

	// Imports the default level txt format and creates a Model from each .h2b
	bool virtual LoadMeshes(const char* gameLevelPath,
		const char* h2bFolderPath,
		GW::SYSTEM::GLog log, GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection) {

		//light stuff RGBA
		sunLightDir = { 1.0f, -1.0f, 2.0f, 0.0f };
		GW::MATH::GVector::NormalizeF(sunLightDir, sunLightDir);

		sunLightColor = { 51.0 / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f };
		sunLightAmbient = { 0.35f, 0.35f, 0.35f };
		sunLight = { sunLightColor, sunLightDir, sunLightAmbient };

		// What this does:
		// Parse GameLevel.txt 
		// For each model found in the file...
			// Create a new Model class on the stack.
				// Read matrix transform and add to this model.
				// Load all CPU rendering data for this model from .h2b
			// Move the newly found Model to our list of total models for the level

		log.Create("../LevelLoaderLog.txt");
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
		while (+file.ReadLine(linebuffer, 1024, '\n'))
		{
			// having to have this is a bug, need to have Read/ReadLine return failure at EOF
			if (linebuffer[0] == '\0')
				break;
			if (std::strcmp(linebuffer, "MESH") == 0)
			{
				Model newModel;
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
				// create the model file name from this (strip the .001)
				newModel.SetName(linebuffer);
				std::string modelFile = linebuffer;
				modelFile = modelFile.substr(0, modelFile.find_last_of("."));
				modelFile += ".h2b";

				//if the name of model is skybox, then we need to get the custom attribute
				if (newModel.name == "skyBox") {
					file.ReadLine(linebuffer, 1024, '\n');

					std::string filepath = linebuffer;

					newModel.skyBox = linebuffer;
				}

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
					allObjectsInLevel.push_back(std::move(newModel));
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

			//load lights
			else if (std::strcmp(linebuffer, "LIGHT") == 0)
			{
				LIGHT_DATA lightdata; //used for the buffer
				Light light;		  //used for tracking

				//get light name and assign
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("LIGHT Detected: ") + linebuffer).c_str());

				light.SetName(linebuffer);

				//read light type and assign it
				file.ReadLine(linebuffer, 1024, '\n');


				if (std::strcmp(linebuffer, "POINT") == 0) {

					light.SetType("POINT");
					light.blend = -1.0f;
					light.size = -1.0f;
				}

				else if (std::strcmp(linebuffer, "SPOT") == 0) {

					light.SetType("SPOT");
				
				}

				log.LogCategorized("INFO", (std::string("LIGHT TYPE: ") + linebuffer).c_str());

				//read color from file
				file.ReadLine(linebuffer, 1024, '\n');

				std::string colorString = linebuffer;

				size_t R = colorString.find("r="); //find where the colors are located in the line
				size_t G = colorString.find("g=");
				size_t B = colorString.find("b=");

				std::string r = colorString.substr(R + 2, 6);  //seperate into seperate strings
				std::string g = colorString.substr(G + 2, 6);
				std::string b = colorString.substr(B + 2, 6);

				float red, green, blue;

				std::stringstream(r) >> red; //convert to float
				std::stringstream(g) >> green;
				std::stringstream(b) >> blue;

				//Assign to object
				GW::MATH::GVECTORF colorVector = { red, green, blue, 1.0 };
				light.SetColor(colorVector);

				log.LogCategorized("INFO", (std::string("LIGHT COLOR: ") + linebuffer).c_str());

				//read energy from file
				file.ReadLine(linebuffer, 1024, '\n');

				float power;
				std::stringstream(linebuffer) >> power;
				light.SetIntensity(power);

				log.LogCategorized("INFO", (std::string("LIGHT POWER: ") + linebuffer).c_str());
				log.LogCategorized("INFO", (std::string("LIGHT POWER: Converting to Intensity... ")).c_str());

				//read radius
				file.ReadLine(linebuffer, 1024, '\n');

				float radius;
				std::stringstream(linebuffer) >> radius;
				light.radius = radius;

				log.LogCategorized("INFO", (std::string("LIGHT RADIUS: ") + linebuffer).c_str());

				if (light.type == "SPOT")
				{
					//read size 
					file.ReadLine(linebuffer, 1024, '\n');

					float spotsize;
					std::stringstream(linebuffer) >> spotsize;
					light.size = spotsize;

					log.LogCategorized("INFO", (std::string("LIGHT SPOT SIZE: ") + linebuffer).c_str());


					//read size 
					file.ReadLine(linebuffer, 1024, '\n');

					float spotblend;
					std::stringstream(linebuffer) >> spotblend;
					light.blend = spotblend;

					log.LogCategorized("INFO", (std::string("LIGHT SPOT BLEND: ") + linebuffer).c_str());

				}

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

				GW::MATH::GVECTORF position = transform.row4;
				light.setPosition(position);

				//place into lights vector
				lights.push_back(std::move(light));

				log.LogCategorized("MESSAGE", "Light Imported successfully");
			}
		}

		//Now we copy the data we can send to the shader to a seperate vector (for lights)

		if (lights.size() > 16)
		{
			log.LogCategorized("ERROR", "You have more lights in the level than are currently supported. Only the first 16 will be supported");
		}

		for (int i = 0; i < 16; i++)
		{
			//break the loop if all lights were imported
			if (i >= lights.size())
			{
				log.LogCategorized("MESSAGE", "All lights have been imported");
				break;
			}

			LIGHT_DATA shaderLight;

			shaderLight.color = lights[i].color;
			shaderLight.blend = lights[i].blend;
			shaderLight.intensity = (int)lights[i].intensity;
			shaderLight.radius = lights[i].radius;
			shaderLight.size = lights[i].size;
			shaderLight.position = lights[i].position;

			LIGHTDATA.push_back(std::move(shaderLight));
			ubo.numLights++;
		}

		log.LogCategorized("MESSAGE", "The first 16 lights have been imported");
		log.LogCategorized("MESSAGE", "Game Level File Reading Complete.");
		// level loaded into CPU ram
		log.LogCategorized("EVENT", "GAME LEVEL WAS LOADED TO CPU [OBJECT ORIENTED]");

	
		

		return true;
	}
	// Upload the CPU level to GPU
	void UploadLevelToGPU(GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection) {
		// iterate over each model and tell it to draw itself
		for (auto& e : allObjectsInLevel) {
			e.UploadModelData2GPU(_ogl, _camera, _view, _projection, sunLight, LIGHTDATA);
		}

	}
	// Draws all objects in the level
	void Render(GW::GRAPHICS::GOpenGLSurface _ogl, GW::MATH::GMATRIXF _camera, GW::MATH::GMATRIXF _view, GW::MATH::GMATRIXF _projection) {

		// iterate over each model and tell it to draw itself
		for (auto& e : allObjectsInLevel) {
			e.DrawModel(_ogl, _camera, _view, _projection, sunLight, LIGHTDATA);

		}

		
	}


	// used to wipe CPU & GPU level data between levels
	void UnloadLevel() {
		allObjectsInLevel.clear();
	}
	// *THIS APPROACH COMBINES DATA & LOGIC* 
	// *WITH THIS APPROACH THE CURRENT RENDERER SHOULD BE JUST AN API MANAGER CLASS*
	// *ALL ACTUAL GPU LOADING AND RENDERING SHOULD BE HANDLED BY THE MODEL CLASS* 
	// For example: anything that is not a global API object should be encapsulated.


};


