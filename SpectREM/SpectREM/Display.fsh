#version 330
in vec2 UV;
out vec3 color;
uniform sampler2D mySampler;

void main()
{
    // Flip the texture coordinates or the image draws upside down
    vec2 texCoord = UV * vec2(1.0, -1.0);
    
    float c = texture( mySampler, texCoord).r * 255;
    
    if (c == 0)
    {
        color = vec3(0, 0, 0);
    }
    else if (c == 1)
    {
        color = vec3(0, 0, 0.8153);
    }
    else if (c == 2)
    {
        color = vec3(0.8153, 0, 0);
    }
    else if (c == 3)
    {
        color = vec3(0.8153, 0, 0.8153);
    }
    else if (c == 4)
    {
        color = vec3(0, 0.8153, 0);
    }
    else if (c == 5)
    {
        color = vec3(0, 0.8153, 0.8153);
    }
    else if (c == 6)
    {
        color = vec3(0.8153, 0.8153, 0);
    }
    else if (c == 7)
    {
        color = vec3(0.8153, 0.8153, 0.8153);
    }

    if (c == 8)
    {
        color = vec3(0, 0, 0);
    }
    else if (c == 9)
    {
        color = vec3(0, 0, 1);
    }
    else if (c == 10)
    {
        color = vec3(1, 0, 0);
    }
    else if (c == 11)
    {
        color = vec3(1, 0, 1);
    }
    else if (c == 12)
    {
        color = vec3(0, 1, 0);
    }
    else if (c == 13)
    {
        color = vec3(0, 1, 1);
    }
    else if (c == 14)
    {
        color = vec3(1, 1, 0);
    }
    else if (c == 15)
    {
        color = vec3(1, 1, 1);
    }
    
}
