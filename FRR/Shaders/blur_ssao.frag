#version 330 core
out vec4 FragColor;


uniform sampler2D ssaoInput;
uniform float width;
uniform float height;
void main()
{
     vec2 texCoords= vec2(gl_FragCoord.x/width,gl_FragCoord.y/height);
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, texCoords + offset).r;
        }
    }
    float val = result / (4.0 * 4.0);
    FragColor = vec4(val, val, val, 1.0);
}
