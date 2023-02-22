#version 460 core
in float id;
out vec4 FragColor;

const float MAX_TRAJ_NUM = 100.0;

void main()
{
    float val = id / MAX_TRAJ_NUM;
    FragColor = vec4(val, 1-val, 0, 1);//但是理论上，其实仅仅需要val值，其他的都可以为0；
    //可视化小技巧，如果想要每个像素都有值，就使用互补的像素值，此消彼长
};