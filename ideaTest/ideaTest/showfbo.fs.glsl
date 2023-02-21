#version 460 core
uniform sampler2D trajFBO;
in vec2 ct;
out vec4 fragcolor;

void main(){
	fragcolor = texture2D(trajFBO, ct);
}
