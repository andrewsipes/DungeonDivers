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

uniform bool isSkybox;
uniform samplerCube skybox;

in vec3 worldNorm;
in vec3 worldPos;
in vec3 coords;
out vec4 Pixel;

uniform bool isUi;

void main() 
{   
   if (isSkybox) {
        
        Pixel = texture(skybox, coords);
        return;
    }

   else if(isUi){

    Pixel = vec4(ubo.material.Kd, 1.0);
   }

   else {
      
   // ---- POINT / SPOT LIGHT -----
 
    vec3 totalColor;

     for(int i = 0; i < ubo.numLights; i++){
        
         //POINT
         if (lbo.lights[i].blend == -1){
        
               //attenuation
               float dist = length(lbo.lights[i].pos.xyz - worldPos);
               float attenuation =  1.0 - clamp((dist)/ lbo.lights[i].radius, 0.0, 1.0);

               //pointlight
               vec3 lightDir = normalize(lbo.lights[i].pos.xyz - worldPos);
               float lightRatio = clamp(dot(lightDir, worldNorm), 0.0, 1.0);
   
               //half-vector 
               vec3 viewDir = normalize(lbo.lights[i].pos.xyz- worldPos);
               vec3 halfvector = normalize(lightDir + viewDir);
               float intensity = max(pow(clamp(dot(worldNorm, halfvector), 0.0, 1.0), ubo.material.Ns), 0.0);
               vec3 pointReflected = lbo.lights[i].color.xyz * ubo.material.Ks * intensity;
      
               vec3 lightsColor = lightRatio * lbo.lights[i].color.xyz * lbo.lights[i].intensity * ubo.material.Kd * attenuation * attenuation;
               //lightsColor += pointReflected;
               totalColor += lightsColor;

          }

          //SPOT
          else{

              float dist = length(lbo.lights[i].pos.xyz - worldPos);
              vec3 coneDir = normalize(worldPos - lbo.lights[i].pos.xyz);
              vec3 lightDir = normalize(lbo.lights[i].pos.xyz - worldPos);
              float surfaceRatio = clamp(dot(-lightDir, coneDir), 0.0, 1.0);
              float lightRatio = clamp(dot(lightDir, worldNorm), 0.0, 1.0);

              float innerConeRatio = cos(lbo.lights[i].size);
              float outerConeRatio = cos(lbo.lights[i].size + lbo.lights[i].blend);   

              float spotFactor = (surfaceRatio > innerConeRatio) ? 1.0 : 0.0;
              float attenuation = 1.0 - clamp((innerConeRatio - surfaceRatio) / (innerConeRatio - outerConeRatio) , 0.0, 1.0);
              float rattenuation =  1.0 - clamp((dist)/ lbo.lights[i].radius, 0.0, 1.0);

              vec3 lightsColor = spotFactor * lightRatio * lbo.lights[i].color.xyz * lbo.lights[i].intensity * ubo.material.Kd * attenuation * rattenuation;
              totalColor += lightsColor;

          }


      }
 

       // ----- SUN -----
       //lambertian
       float _dot = dot(-ubo.sunDirection.xyz, worldNorm); 
       float sunlightRatio = clamp(_dot, 0.0, 1.0);
       vec3 sunColor = (ubo.sunColor.xyz);
       sunColor *= sunlightRatio * ubo.material.Kd;

       //ambient
       vec3 ambientColor = ubo.sunAmbient.xyz * ubo.material.Kd;

       //half-vector 
       vec3 camWorldPos = ubo.cMatrix[3].xyz;
       vec3 sunViewDir = normalize(camWorldPos - worldPos);
       vec3 sunHalfvector = normalize(-ubo.sunDirection.xyz + sunViewDir);
       float sunIntensity = max(pow(clamp(dot(worldNorm, sunHalfvector), 0.0, 1.0), ubo.material.Ns), 0.0);
       vec3 sunReflected = ubo.sunColor.xyz * ubo.material.Ks * sunIntensity;

       // ----- FINAL -----

       Pixel = vec4(totalColor + sunColor + sunReflected + ambientColor, 1.0);
    }
}