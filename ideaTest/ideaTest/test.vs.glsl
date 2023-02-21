#version 460 core
layout (location = 0) in vec2 aPos;

uniform float MAXX;
uniform float MINX;
uniform float MAXY;
uniform float MINY;

void main(){
	float x = ((aPos.x - MINX) / (MAXX - MINX) - 0.5) * 2;
	float y = ((aPos.y - MINY) / (MAXY - MINY) - 0.5) * 1.5;
	gl_Position = vec4(x, y, 0.0, 1.0);
}
