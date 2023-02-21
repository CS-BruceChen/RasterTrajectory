#version 460 core
in vec2 ct;
out vec4 FragColor;

uniform sampler2D trajFBO;

void main()
{
    //FragColor = texelFetch(trajFBO, ivec2(gl_FragCoord.xy), 0);
    FragColor=vec4(texture(trajFBO,ct).rgb,1.0);
}