#version 330 core

in vec2 uv;

uniform sampler2D rock;
uniform sampler2D grass;
uniform sampler2D snow;

in float steepness;
in float height;

// Ouput data
out vec3 color;


const float transition_1 = 0.14;
const float transition_2 = 0.2;



void main(){

        
        color = mix(
                texture(grass, uv).rgb,
                texture(rock, uv).rgb,
                smoothstep(0, transition_1, clamp(height, 0.0, transition_1) )
        );

        color = mix(
                color,
                texture(snow, uv).rgb,
                smoothstep(0, transition_2-transition_1, clamp(height-transition_1, 0, transition_2-transition_1))
        );

        float k = clamp(height / 10.0, 3, 20); // on veut mettre davantage de rochers près du sol.
        color = mix(color, texture(rock, uv).rgb, clamp(steepness / k, 0, 1));


}
