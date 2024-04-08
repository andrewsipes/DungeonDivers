#pragma pack_matrix(row_major)

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
	float4x4 matrices[MAX_SUBMESH_PER_DRAW];
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

//// For push_constants to work in HLSL you must prepend this to a cbuffer
//struct MESH_INDEX {
//	uint mesh_ID, model_ID;
//};
//[[vk::push_constant]]
//MESH_INDEX offset;

[[vk::push_constant]]
cbuffer MESH_INDEX {
	uint mesh_ID, model_ID;
};

struct V_OUT
{
	float4 pos : SV_POSITION;
	float3 nrm : NORMAL;
	float3 posW : WORLD;
};

V_OUT main(float3 pos : POSITION, float3 uvw : TEXCOORD, float3 nrm : NORMAL, uint index : SV_InstanceID)
{
	/*V_OUT output = { SceneData[0].matrices[0][0], nrm, uvw };
	return output;*/

	matrix pivot = SceneData[model_ID].matrices[mesh_ID];
	matrix world = SceneData[model_ID].instances[index];
	matrix final = mul(pivot, world);
		
	float4 posW = mul(float4(pos, 1), final);
	float4 posH = mul(posW, viewMatrix);
	posH = mul(posH, projectionMatrix);
	
	V_OUT output = { posH, mul(float4(nrm,0), final).xyz, posW.xyz };
	return output;
}