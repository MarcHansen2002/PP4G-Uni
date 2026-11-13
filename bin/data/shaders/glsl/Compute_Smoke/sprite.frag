// frag shader
#version 430


layout (set = 1, binding = 0) uniform sampler2D tex_sampler;


// input
layout (location = 0) in vec2 in_tex_coord;

// output
layout (location = 0) out vec4 frag_colour;


void main ()
{

    vec4 smoke = texture(tex_sampler, in_tex_coord);
    float total = smoke.r + smoke.g + smoke.b;
    vec3 colour = vec3(0.0);
    if (total > 0.0001)
    {
        colour = smoke.rgb / total;
    }

    frag_colour = vec4(colour, clamp(smoke.a, 0.0, 1.0));
    
  // frag_colour = vec4(
  //     clamp(density, 0.0, 1.0),
  //     0.0,
  //     0.0,
  //     1.0
  // );

  //frag_colour = texture (tex_sampler, in_tex_coord);
  //frag_colour = vec4(frag_colour.x, frag_colour.x, frag_colour.x, 1);
}
