#version 460 core

uniform ivec4 A;
uniform ivec4 B;

layout(std430, binding = 0) buffer resultBuf{
	int result[];
};

//Declare what size is the group;
layout (local_size_x = 1) in;

void main(){
	int val = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
	atomicAdd(result[0],val);
}

