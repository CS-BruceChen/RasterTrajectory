#version 460 core
#define BLOCKSIZE 518
//但是在编译时这里可以变成任意的值,通过外部修改字符串。
layout(binding = 0, r32i) uniform iimageBuffer DPBuf;
layout (local_size_x = BLOCKSIZE, local_size_y = 1) in;

void main(){
	
}