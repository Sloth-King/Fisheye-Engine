#version 330 core

uniform mat4 MVP;
uniform vec3 viewVector;


in vec2 uv;

out vec3 color;

void main(){
        color = vec3(0.8, 0.6, 0.2);
}

const vec3 lightdirection = normalize(vec3(1, 1, 1));

void main() {
    
    //Calculate light direction and view direction.
    vec3 viewDirection = normalize(viewPosition - vertPos);
    
    //Cosine theta diffuse lambertian component.
    float cosTheta = max(0.0, dot(normalInterp, normalize(lightDirection)));
    
    vec4 emissive = surfaceMaterial.ke * light.color;
    vec4 ambient = surfaceMaterial.ka * light.color;
    vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
    
    //Only if light is visible from the surface point.
    if(cosTheta > 0.0) {
        
        //Reflection vector around normal.
        vec3 reflectionDirection = reflect(-lightDirection, normalInterp);
        
        //Diffuse component.
        diffuse = surfaceMaterial.kd * light.color * cosTheta;
        
        //Specular component.
        specular = surfaceMaterial.ks * light.color * pow(max(0.0, dot(reflectionDirection, viewDirection)), surfaceMaterial.sh);
    }
    
    fragmentColor = emissive + ambient + diffuse + specular;
}