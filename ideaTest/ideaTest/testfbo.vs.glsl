#version 460 core
layout (location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;
//uniform float MAXX;
//uniform float MINX;
//uniform float MAXY;
//uniform float MINY;
out vec2 ct;

void main(){
	//float x = ((aPos.x - MINX) / (MAXX - MINX) - 0.5) * 2;
	//float y = ((aPos.y - MINY) / (MAXY - MINY) - 0.5) * 1.5;
	gl_Position = vec4(aPos.x/1.2, aPos.y/1.2, 0.0, 1.0);
	ct=aTex;
}