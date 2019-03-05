#version 330 core

out vec4 frag_color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform float width;
uniform float height;
uniform vec3 samples[64];

uniform float radius;
uniform float bias;

int kernelSize = 64;


void main(){

    vec2 texCoords= vec2(gl_FragCoord.x/width,gl_FragCoord.y/height);

    float d=texture(gPosition,texCoords).z;
    vec3 n =texture(gNormal,texCoords).xyz*2-vec3(1);
    float occlusion =0;

   for(int i = 0; i<kernelSize;i++){
        float d_p=texture(gPosition,texCoords+samples[i].xy*radius/width).z;
        vec3 n_p =texture(gNormal,texCoords+samples[i].xy*radius/width).xyz*2-vec3(1);
        if (d_p>d+bias){
           occlusion +=1-dot(n,n_p);
        }

    }
    occlusion=1-occlusion/kernelSize;
    frag_color=vec4(occlusion,occlusion,occlusion,1);
}
