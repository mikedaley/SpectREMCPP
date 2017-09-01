//
//  Shader.fsh
//  SpectREM
//
//  Created by Michael Daley on 31/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

void main()
{
    const float w = 320;
    const float h = 256;
    
    vec2 vUv = v_tex_coord.xy * vec2(w, h);
    
    vec2 alpha = vec2(0.25);
    vec2 x = fract(vUv);
    vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
    
    vec2 texCoord = (floor(vUv) + x_) / vec2(w, h);
    gl_FragColor = texture2D(u_texture, texCoord);
}
