#version 460 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoTex;
uniform sampler2D gRoughnessMetallicTex;
uniform sampler2D gDepth;

uniform vec3 view;
uniform Mat4 VP;

const float PI = 3.14159265359;
const int atlas_nb_textures = 8;
const int atlas_size_textures = 8;


const int num_lights = PLACEHOLDER_num_lights; //autoreplaced by the renderingserver at compilation

struct Light{ // order is important. See Light class
   vec3 position;
   uint type;
   vec3 color;
   float intensity;
   float radius;
};

uniform int currentLightCount;

layout(std140) uniform lights // can it be the same name?
{
   Light lights[num_lights]; 
}

in vec2 uv;

out vec3 color;

// PBR implementation by andrew

float DistributionGGX(vec3 N, vec3 H, float a)
{
   float a2     = a*a;
   float NdotH  = max(dot(N, H), 0.0);
   float NdotH2 = NdotH*NdotH;
  
   float nom    = a2;
   float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
   denom        = PI * denom * denom;
  
   return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
   float nom   = NdotV;
   float denom = NdotV * (1.0 - k) + k;
  
   return nom / denom;
}
 float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
   float NdotV = max(dot(N, V), 0.0);
   float NdotL = max(dot(N, L), 0.0);
   float ggx1 = GeometrySchlickGGX(NdotV, k);
   float ggx2 = GeometrySchlickGGX(NdotL, k);
  
   return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
   return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


vec3 pbr_lighting(vec3 albedo, vec3 normal, float roughness, float metallic, Light light_struct){

   vec3 N = normalize(normal); 
   vec3 V = normalize(view);

   vec3 light = light_struct.lightDir;
   vec3 lightColor = light_struct.lightColor;

   vec3 Lo = vec3(0.0);

   //make a for loop if we have more than 1 light probably
   vec3 L = normalize(light);
   vec3 H = normalize(V + L); //half vector
   
   float distance = length(light);
   float attenuation = 1.0 / (distance * distance);
   vec3 radiance = lightColor * attenuation; 

   vec3 F0 = vec3(0.04); 
   F0 = mix(F0, albedo, metallic);
   vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
   float NDF = DistributionGGX(N, H, roughness);       
   float G = GeometrySmith(N, V, L, roughness);   

   vec3 numerator = NDF * G * F;
   float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
   vec3 specular = numerator / denominator; 

   vec3 kS = F;
   vec3 kD = vec3(1.0) - kS;
   
   kD *= 1.0 - metallic;	

   float NdotL = max(dot(N, L), 0.0);                
   Lo += (kD * albedo / PI + specular) * radiance * NdotL; //+= cuz its supposed to be multiple lights

   //end of for loop

   vec3 ambient = vec3(0.03) * albedo; // * ao if we create an ao map but tbh idgaf
   vec3 tempColor = ambient + Lo;

   tempColor = tempColor / (tempColor + vec3(1.0));
   return pow(tempColor, vec3(1.0/2.2));  

}

void main(){    

   color = vec3(0);

   vec3 position = texture(gPosition, uv).xyz;
   vec3 normal = texture(gNormal, uv).xyz;


   vec4 albedo = texture(gAlbedoTex, uv);
   vec3 textureNormal = texture(gNormal, uv).xyz;
   float roughness = texture(gRoughnessMetallicTex, uv).x;
   float metallic = texture(gRoughnessMetallicTex, uv).y;

   float depth = texture(gDepth, uv).x;

   float zNear = 0.1;
   float  zFar = 300.0;

   lights[0] = sun;

   //Our roughness map is brifhtness map 
   roughness = 1 - roughness; 

   //calculating our albedo map

   if (depth >= 1.0){ // means we're in the skybox
      color = albedo.xyz;
   } else {
      for(int i = 0 ; i < currentLightCount ; i++){
         color+= pbr_lighting(albedo.xyz, normal, roughness, metallic, lights[i]);
      }
   }


   // light fog bloom
   for(int i = 0 ; i < currentLightCount ; i++){
      Light light = lights[i];

      vec2 posClipSpace = VP * glm::vec4(light.position, 1.0).xy;
      float d = distance(posClipSpace, uv);
      if (d <= 0.1){
         color += light.Color * 1.0/pow(d, 2);
      }
   }
   //depth =  zNear * zFar / (zFar + depth * (zNear - zFar)) / 10.0;

   color = mix(
      color,
      vec3(0.05, 0.20, 0.14),
      min(1.2 * pow(depth, 10), 1.0)
   );
} 
)