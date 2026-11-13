// frag shader
#version 430


layout (set = 1, binding = 0) uniform sampler2D tex_sampler;


// input
layout (location = 0) in vec2 in_tex_coord;

// output
layout (location = 0) out vec4 frag_colour;


void main ()
{

    float density = texture(tex_sampler, in_tex_coord).r;
    
    frag_colour = vec4(
        clamp(density, 0.0, 1.0),
        0.0,
        0.0,
        1.0
    );

  //frag_colour = texture (tex_sampler, in_tex_coord);
  //frag_colour = vec4(frag_colour.x, frag_colour.x, frag_colour.x, 1);
}
