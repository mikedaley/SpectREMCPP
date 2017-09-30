#version 330
in vec2 UV;
out vec4 color;
uniform sampler2D mySampler;

void main()
{
    const float w = 320;
    const float h = 256;
    const float border = 32 - 32;
    
    const float new_w = w - (border * 2);
    const float new_h = h - (border * 2);
    
    vec2 texCoord = UV * vec2(1.0, -1.0);
    
    float u = ((texCoord.x * new_w) + border);
    float v = ((texCoord.y * new_h) - border);

    vec2 vUv = vec2(u, v);
    vec2 alpha = vec2(0.0); // 0.5 = Linear, 0.0 = Nearest
    vec2 x = fract(vUv);
    vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
    
    texCoord = (floor(vUv) + x_) / vec2(w, h);

    float c = texture( mySampler, texCoord).r * 255;
    
    if (c == 0)
    {
        color = vec4(0, 0, 0, alpha);
    }
    else if (c == 1)
    {
        color = vec4(0, 0, 0.8153, alpha);
    }
    else if (c == 2)
    {
        color = vec4(0.8153, 0, 0, alpha);
    }
    else if (c == 3)
    {
        color = vec4(0.8153, 0, 0.8153, alpha);
    }
    else if (c == 4)
    {
        color = vec4(0, 0.8153, 0, alpha);
    }
    else if (c == 5)
    {
        color = vec4(0, 0.8153, 0.8153, alpha);
    }
    else if (c == 6)
    {
        color = vec4(0.8153, 0.8153, 0, alpha);
    }
    else if (c == 7)
    {
        color = vec4(0.8153, 0.8153, 0.8153, alpha);
    }

    if (c == 8)
    {
        color = vec4(0, 0, 0, alpha);
    }
    else if (c == 9)
    {
        color = vec4(0, 0, 1, alpha);
    }
    else if (c == 10)
    {
        color = vec4(1, 0, 0, alpha);
    }
    else if (c == 11)
    {
        color = vec4(1, 0, 1, alpha);
    }
    else if (c == 12)
    {
        color = vec4(0, 1, 0, alpha);
    }
    else if (c == 13)
    {
        color = vec4(0, 1, 1, alpha);
    }
    else if (c == 14)
    {
        color = vec4(1, 1, 0, alpha);
    }
    else if (c == 15)
    {
        color = vec4(1, 1, 1, alpha);
    }
}
