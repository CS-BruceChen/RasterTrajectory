#version 460 core
in float id;
out vec4 FragColor;

const float MAX_TRAJ_NUM = 100.0;

void main()
{
    float val = id / MAX_TRAJ_NUM;
    FragColor = vec4(val, 1-val, 0, 1);//���������ϣ���ʵ������Ҫvalֵ�������Ķ�����Ϊ0��
    //���ӻ�С���ɣ������Ҫÿ�����ض���ֵ����ʹ�û���������ֵ�������˳�
};