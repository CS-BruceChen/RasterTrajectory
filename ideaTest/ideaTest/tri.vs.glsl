#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in float trajectoryId;
out float id;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    id = trajectoryId;
}