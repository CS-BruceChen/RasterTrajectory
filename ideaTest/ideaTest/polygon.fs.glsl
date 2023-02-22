#version 460 core
out vec4 FragColor;
uniform sampler2D trajFBO;

uniform layout(binding = 0, r32i) iimageBuffer resultBuf;

const float MAX_TRAJ_NUM = 100.0;

int getId(float val){
    int id = int(round(val * MAX_TRAJ_NUM));
    return id;
}

void main()
{
    //exactlly, only the pixel in the polygon will be fetch here.
    vec4 pix = texelFetch(trajFBO, ivec2(gl_FragCoord.xy), 0);
    int id = getId(pix.r);
    imageAtomicAdd(resultBuf, id, 1);
    FragColor = vec4(pix.rgb,1.0);
}