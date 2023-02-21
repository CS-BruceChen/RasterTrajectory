#version 460 core
out vec4 FragColor;
uniform sampler2D trajFBO;

void main()
{
    vec4 pix = texelFetch(trajFBO, ivec2(gl_FragCoord.xy), 0);
    FragColor = vec4(pix.rgb,1.0);
    
}
