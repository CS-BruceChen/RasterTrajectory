#version 460 core
#define BLOCKSIZE 518
//�����ڱ���ʱ������Ա�������ֵ,ͨ���ⲿ�޸��ַ�����
layout(binding = 0, r32i) uniform iimageBuffer DPBuf;
layout (local_size_x = BLOCKSIZE, local_size_y = 1) in;

void main(){
	
}