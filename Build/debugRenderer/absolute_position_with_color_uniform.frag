#version 330 core

out vec4 frag_color;

uniform vec4 color;
 
void main()
{
     frag_color = color;
//    frag_color = vec4(0.5, 1.0, 0.0, 1.0);
}
