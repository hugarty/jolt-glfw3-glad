#version 330 

in vec2 passthrough_texture_coordinate;

uniform sampler2D texture_sampler;

out vec4 frag_color;

void main()
{    
    frag_color = texture(texture_sampler, passthrough_texture_coordinate);
}
