// This file is a Cpp header file that contains the raw code for the shaders as strings.
// the extension is just for syntax highlighting. Don't try to shader compile that shit
#include <string>


std::string vertex_deferred= R"( // vertex deferred start
#version 460 core

layout(location = 0) in vec3 RAW_POSITION; // screen space
layout(location = 1) in vec2 RAW_UV;
layout(location = 2) in vec3 RAW_NORMAL;
layout(location = 3) in vec3 RAW_TANGENT;
layout(location = 4) in vec3 RAW_BITANGENT;

uniform mat4 MVP;
uniform mat4 MODEL;

out float DEPTH;
out vec2 UV;
out vec3 POSITION;
out mat3 TBN;


void main () {
   gl_Position = MVP * vec4(RAW_POSITION, 1);
   DEPTH = gl_Position.w;

   UV = RAW_UV;

   POSITION = ( MODEL * vec4(RAW_POSITION, 1)).xyz;

   TBN = mat3(
      (MODEL * vec4(RAW_TANGENT, 0)).xyz, 
      (MODEL * vec4(RAW_BITANGENT, 0)).xyz, 
      (MODEL * vec4(RAW_NORMAL, 0)).xyz
   );

}
)"; // vertex deferred end




std::string fragment_deferred= R"( // fragment deferred start
#version 460 core
layout (location = 0) out vec3 _Position;
layout (location = 1) out vec3 _Normal;
layout (location = 2) out vec4 _AlbedoTex;
layout (location = 3) out vec2 _RoughnessMetallicTex;


uniform sampler2D _tex_diffuse;
uniform sampler2D _tex_normal;
uniform sampler2D _tex_roughness;
uniform sampler2D _tex_metallic;

in float DEPTH;
in vec2 UV;
in mat3 TBN;
in vec3 POSITION;


void main(){    
   // store the fragment position vector in the first gbuffer texture
   _Position = POSITION;
   // also store the per-fragment normals into the gbuffer

   vec3 textureNormal = texture(_tex_normal, UV).xyz;

   if (textureNormal != vec3(0, 0, 0)){
      textureNormal = textureNormal * 2.0 - 1.0;   
      _Normal = normalize(TBN * textureNormal);
   } else {
   _Normal = normalize(TBN[2]);
   }


   // and the diffuse per-fragment color
   _AlbedoTex = texture(_tex_diffuse, UV);

   _RoughnessMetallicTex.r = texture(_tex_roughness, UV).r;
   _RoughnessMetallicTex.g = texture(_tex_metallic, UV).r;

}  
)"; // fragment deferred end



std::string PBR_vertex_deferred= R"( // vertex PBR deferred start
#version 460 core

layout(location = 0) in vec2 RAW_POSITION;

out vec2 uv;

void main () {
   gl_Position = vec4(RAW_POSITION, 0, 1);

   uv = (RAW_POSITION + vec2(1.0)) / 2.0;
}
)"; // vertex deferred end

std::string fragment_PBR_deferred= R"( // PBR deferred start
#version 460 core

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoTex;
uniform sampler2D gRoughnessMetallicTex;
uniform sampler2D gDepth;

uniform vec3 view;
uniform mat4 VP;

const float PI = 3.14159265359;
const int atlas_nb_textures = 8;
const int atlas_size_textures = 8;


const int num_lights = PLACEHOLDER_num_lights; //autoreplaced by the renderingserver at compilation

struct Light{ // order is important. See Light class
   vec3 position;
   int type;
   vec3 color;
   float intensity;
   vec3 direction;
   float radius;
};

uniform int currentLightCount;

layout(std140) uniform lightsUniform // can it be the same name?
{
   Light lights[num_lights]; 
};

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

const float falloff_fac = 10.0;
vec3 pbr_lighting(vec3 position, vec3 albedo, vec3 normal, float roughness, float metallic, Light light_struct){

   vec3 N = normalize(normal); 
   vec3 V = normalize(-view);

   vec3 lightColor = light_struct.color * light_struct.intensity;

   albedo = (pow(albedo, vec3(2.2))); // convert from srgb to linear

   vec3 Lo = vec3(0.0);

   vec3 L = normalize( light_struct.position - position );
   vec3 H = normalize(V + L); //half vector
   
   float distance = distance(position, light_struct.position);
   float attenuation = 1.0 / (1.0 + falloff_fac * distance * distance);
   vec3 radiance = lightColor * attenuation; 

   vec3 F0 = vec3(0.04); 
   F0 = mix(albedo, F0, metallic);
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

   //vec3 ambient = vec3(0.03) * albedo; // * ao if we create an ao map but tbh idgaf
   return  Lo;

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

   //Our roughness map is brifhtness map 
   roughness = 1.0 - roughness; 

   //calculating our albedo map

   if (depth >= 1.0){ // means we're in the skybox
      color = albedo.xyz;
   } else {
      for(int i = 0 ; i < currentLightCount ; i++){
         color+= pbr_lighting(position, albedo.xyz, normal, roughness, metallic, lights[i]);

      }
   // fin du pbr. Voir avec andrew
      color = color / (color + vec3(1.0));
      color = pow(color, vec3(1.0/2.2));  
   }

   /*
   // light fog bloom
   for(int i = 0 ; i < currentLightCount ; i++){
      Light light = lights[i];

      vec2 posClipSpace = (transpose(VP) * vec4(light.position, 0.0)).xy;
      posClipSpace = (posClipSpace + 1.0) / 2.0;
      float d = distance(posClipSpace, uv);
      if (d <= 0.1){
         color += clamp(mix(light.color, vec3(0.0), d * d / 0.1), 0.0, 1.0);
      }
   }
   //depth =  zNear * zFar / (zFar + depth * (zNear - zFar)) / 10.0;
   */
   color = mix(
      color,
      vec3(0.05, 0.20, 0.14),
      min(1.2 * pow(depth, 200), 1.0)
   );
}
)"; // PBR deferred end



// https://www.reddit.com/r/opengl/comments/y83zy1/performance_issues_with_deferred_rendering/