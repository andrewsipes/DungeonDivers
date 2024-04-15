#version 330 // GLSL 3.30

struct ATTRIBUTES {
		vec3 Kd; 
        float d;
		vec3 Ks; 
        float Ns;
		vec3 Ka; 
        float sharpness;
		vec3 Tf; 
        float Ni;
		vec3 Ke; 
        uint illum;
	};

struct Light{
    vec4 pos;
    vec4 color;
    int intensity;
    float radius;
    float size;
    float blend;
};

uniform UboData {
    vec4 sunColor;    
    vec4 sunDirection;
    vec3 sunAmbient;
    mat4 cMatrix;
    mat4 vMatrix;
    mat4 pMatrix;
    mat4 wMatrix;
    ATTRIBUTES material;
    int numLights;
} ubo;

uniform lightData {
    Light lights[16];
} lbo;


layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 uv;
layout(location = 2) in vec3 normal;

out vec3 worldNorm;
out vec3 worldPos;
out vec3 coords;

uniform bool isUi;

void main()
{
    if (isUi){

        gl_Position = ubo.pMatrix * ubo.vMatrix * ubo.wMatrix * vec4(pos,1);
        //gl_Position = vec4(pos,1);
    }

    else {

    gl_Position = ubo.pMatrix * ubo.vMatrix * ubo.wMatrix * vec4(pos,1);
    
    coords = pos;

    //Get World Position
    mat3 transposed = mat3(transpose(ubo.wMatrix));
    mat3 inverseTransposed = inverse(transposed);
    vec3 transformed = inverseTransposed * normal;
    worldNorm = normalize(transformed);

    vec4 worldPosition = ubo.wMatrix * vec4(pos, 1.0);
    worldPos = worldPosition.xyz;
    }


}