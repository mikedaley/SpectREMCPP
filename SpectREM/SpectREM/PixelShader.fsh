//
//  Shader.fsh
//  SpectREM
//
//  Created by Michael Daley on 31/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

//void main()
//{
//    const float w = 320;
//    const float h = 256;
//    const float wr = 0.003125 * 32;
//    const float hr = 0.00390625 * 32;
//    
//    vec2 coords = vec2(v_tex_coord.x + wr, v_tex_coord.y + hr);
//    
//    vec2 vUv = coords * vec2(w, h);
//    
//    vec2 alpha = vec2(0.07);
//    vec2 x = fract(vUv);
//    vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
//    
//    vec2 texCoord = (floor(vUv) + x_) / vec2(w, h);
//    
//    gl_FragColor = texture2D(u_texture, texCoord);
//}
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
    const float border = 0;
    const float new_w = w - (border * 2);
    const float new_h = h - (border * 2);
    
    // Get the u coordinate based on the size of the new texture - this is in pixels
    float u = ((v_tex_coord.x * new_w) + border);
    float v = ((v_tex_coord.y * new_h) + border);
    vec2 vUv = vec2(u, v);
    
    vec2 alpha = vec2(0.25);
    vec2 x = fract(vUv);
    vec2 x_ = clamp(0.5 / alpha * x, 0.0, 0.5) + clamp(0.5 / alpha * (x - 1.0) + 0.5, 0.0, 0.5);
    
    vec2 texCoord = (floor(vUv) + x_) / vec2(w, h);
    
    gl_FragColor = texture2D(u_texture, texCoord);
}
