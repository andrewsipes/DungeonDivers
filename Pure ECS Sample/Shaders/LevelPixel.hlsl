// For push_constants to work in HLSL you must prepend this to a cbuffer
[[vk::push_constant]]
cbuffer MESH_INDEX {
	uint mesh_ID, model_ID;
};

struct OBJ_ATTRIBUTES
{
	float3      Kd; // diffuse reflectivity
	float	    d; // dissolve (transparency) 
	float3      Ks; // specular reflectivity
	float       Ns; // specular exponent
	float3      Ka; // ambient reflectivity
	float       sharpness; // local reflection map sharpness
	float3      Tf; // transmission filter
	float       Ni; // optical density (index of refraction)
	float3      Ke; // emissive reflectivity
	float		illum; // illumination model
};

#define MAX_SUBMESH_PER_DRAW 1024
#define MAX_INSTANCE_PER_DRAW 4096 
struct SHADER_MODEL_DATA
{
	float4x4 matricies[MAX_SUBMESH_PER_DRAW];
	OBJ_ATTRIBUTES materials[MAX_SUBMESH_PER_DRAW];
	float4x4 instances[MAX_INSTANCE_PER_DRAW];
};
// This is how you declare and access a Vulkan storage buffer in HLSL
StructuredBuffer<SHADER_MODEL_DATA> SceneData : register(b0);

cbuffer SHADER_SCENE_DATA : register(b1)
{
	float4 sunDirection, sunColor, sunAmbient, camPos;
	float4x4 viewMatrix, projectionMatrix;
};

// an ultra simple hlsl pixel shader
float4 main(float4 pos : SV_POSITION, float3 nrm : NORMAL, float3 posW : WORLD) : SV_TARGET
{
	//return float4(0,0,1,0);
	float4 diffuse = float4(SceneData[model_ID].materials[mesh_ID].Kd, 1);
	float4 specular = float4(SceneData[model_ID].materials[mesh_ID].Ks, 1);
	float4 ambient = float4(SceneData[model_ID].materials[mesh_ID].Ka, 1);
	float4 emissive = float4(SceneData[model_ID].materials[mesh_ID].Ke, 1);
	float lightRatio = saturate(dot(-sunDirection, normalize(nrm)));
	
	float3 viewDir = normalize(camPos.xyz - posW);
	float3 halfVec = normalize((-sunDirection) + viewDir);
	float intense = max(pow(saturate(dot(normalize(nrm), halfVec)), 
		SceneData[model_ID].materials[mesh_ID].Ns), 0);
	float4 reflected = sunColor * specular * intense;

	float4 direct = lightRatio * sunColor;
	float4 indirect = sunAmbient * ambient;

	return saturate(direct + indirect) * diffuse + reflected + emissive;
}