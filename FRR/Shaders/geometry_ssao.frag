#version 330
//out vec4 FragColor;
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 N;
in vec3 V;

void main (void) {
    // store the fragment position vector in the first gbuffer texture
    gPosition = V;
    // also store the per-fragment normals into the gbuffer
    //gNormal = normalize(N);
    gNormal = (normalize(N)+vec3(1))/2;

    gAlbedoSpec.rgb = vec3(0.95, 0.95, 0.95);
 //   FragColor = vec4 (0,1,0,1);
}
