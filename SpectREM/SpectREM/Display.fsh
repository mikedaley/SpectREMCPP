#version 330
in vec2 UV;
out vec3 color;
uniform sampler2D mySampler;

void main()
{
    // Flip the texture coordinates or the image draws upside down
    vec2 texCoord = UV * vec2(1.0, -1.0);
    color = texture( mySampler, texCoord).rgb;
}
