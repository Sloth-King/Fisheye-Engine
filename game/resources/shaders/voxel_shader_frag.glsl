#version 460 core

uniform vec3 view;

const float PI = 3.14159265359;
const int atlas_nb_textures = 8;
const int atlas_size_textures = 8;
const int num_lights = 1;

struct Light{
   uint type;
   vec3 lightDir;
   vec3 lightPos;
   vec3 lightColor;
};

Light lights[num_lights]; 

uniform sampler2D atlas;
uniform sampler2D roughness_map;
uniform sampler2D metallic_map;
uniform sampler2D normal_map;


in vec2 uv;
in mat3 TBN;
//in vec3 normal;

// uniform float metallic;
// uniform float roughness;

//light (for now using placeholder)
//TODO : create and manage lights properly (ie. send them to the shader and manager their number)

Light sun = {0, vec3(1.0,1.0,1.0) , vec3(1.0,1.0,1.0), vec3(23.47, 21.31, 20.79)};
// vec3 light = vec3(1.0,1.0,1.0); //placeholder light pos
// vec3  lightColor  = vec3(23.47, 21.31, 20.79);

out vec3 color;

//Distrubution function
//Source : https://learnopengl.com/PBR/Theory (Completely copy pasted this lmaoo)
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

//mostly from here : https://learnopengl.com/PBR/Lighting
void main(){
   
   color = vec3(0.0,0.0,0.0);

   float roughness = texture(roughness_map, uv).r; //roughness map
   float metallic = texture(metallic_map, uv).r; //metallic map
   
   vec3 normal = texture(normal_map, uv).xyz; //normal map

   normal = normal * 2.0 - 1.0;   
   normal = normalize(TBN * normal); //convert to tangent spage


   //https://learnopengl.com/Advanced-Lighting/Normal-Mapping
   //set the normal map to the world space normal
   
   lights[0] = sun;

   //Our roughness map is brifhtness map 
   roughness = 1 - roughness; 


   //calculating our albedo map
   vec3 albedo = texture(atlas, uv).xyz;

   for(int i = 0 ; i < num_lights ; i++){
      color+= pbr_lighting(albedo, normal, roughness, metallic, lights[i]);
   }
   
}