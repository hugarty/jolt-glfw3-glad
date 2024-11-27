#version 330 core

// This shader draws vertices at an absolute location rather than using a
// local to world coordinate transformation

attribute vec3 position;
 
void main()
{
    gl_Position = vec4(position, 1.0f);
}
