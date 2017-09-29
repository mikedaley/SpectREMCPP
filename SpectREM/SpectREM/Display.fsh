#version 330
in vec2 UV;
out vec3 color;
uniform sampler2D mySampler;
uniform sampler2D samp;

void main()
{
    vec2 texCoord = UV * vec2(1.0, -1.0);
    color = texture( mySampler, texCoord).rgb;
}
