
struct SUNLIGHT_DATA
{
	GW::MATH::GVECTORF color, direction, ambient;
};

class Light {

public:
	std::string name;
	std::string type;
	GW::MATH::GVECTORF position;
	GW::MATH::GVECTORF color;
	float intensity;
	float radius;
	float size;
	float blend;

	inline void SetName(std::string lightName) {
		name = lightName;
	}

	inline void SetType(std::string lightType) {
		type = lightType;
	}

	inline void setPosition(GW::MATH::GVECTORF lightPosition) {
		position = lightPosition;
	}

	inline void SetColor(GW::MATH::GVECTORF lightColor) {
		color = lightColor;
	}

	inline void SetPos(GW::MATH::GVECTORF lightPos) {
		position = lightPos;
	}

	//takes in light power, and converts it to intensity
	inline void SetIntensity(float lightPower) {
		intensity = lightPower;
	}

};

//this is what we can send to the shader for lighting
struct LIGHT_DATA
{
	GW::MATH::GVECTORF position;
	GW::MATH::GVECTORF color;
	int intensity;
	float radius;
	float size = -1;
	float blend = -1;

};

//uniform buffer data
struct UBO_DATA
{
	GW::MATH::GVECTORF sunColor, sunDirection, sunAmbient;
	GW::MATH::GMATRIXF _cam, _view, _proj, _world;
	H2B::ATTRIBUTES material;
	int numLights;

} ubo;

//vector of lights - this will be sent to the uniform 
std::vector<LIGHT_DATA> lbo;

//holds the textureID for the skybox
unsigned int CubeMapTexture;

struct Vertex
{
	float  x, y, z, w;
};