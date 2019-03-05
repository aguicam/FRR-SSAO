#version 330 core

out vec4 frag_color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform float width;
uniform float height;

uniform float radius;
uniform float bias;

int kernelSize = 16;


void main(){
    vec2 noiseScale = vec2(width/4.0, height/4.0);
    vec2 texCoords= vec2(gl_FragCoord.x/width,gl_FragCoord.y/height);



    float d=texture(gPosition,texCoords).z;
    vec3 n =normalize(texture(gNormal,texCoords).xyz*2-vec3(1));

    vec3 random_dir = normalize(texture(texNoise, texCoords * noiseScale).xyz);
    random_dir= reflect(random_dir,n);


    float occlusion =0;

    for (int i = -4; i <= 4; ++i) {
      if (i != 0) {
       vec2 texCoords_p = texCoords + random_dir.xy * i*radius/width;
        vec3 n_p =normalize(texture(gNormal,texCoords_p).xyz*2-vec3(1));
        float d_p=texture(gPosition,texCoords_p).z;
        if (d_p>d+bias){
          occlusion +=1-abs(dot(n,n_p));
        }
      }
    }

    for (int i = -4; i <= 4; ++i) {
      if (i != 0) {
        vec2 texCoords_p = texCoords + vec2(-random_dir.y, random_dir.x) * i * radius/width;
        vec3 n_p =normalize(texture(gNormal,texCoords_p).xyz*2-vec3(1));
        float d_p=texture(gPosition,texCoords_p).z;
        if (d_p>d+bias){
          occlusion +=1-abs(dot(n,n_p));
        }
      }
    }

    occlusion=1-occlusion/(kernelSize);
    frag_color=vec4(occlusion,occlusion,occlusion,1);
}
