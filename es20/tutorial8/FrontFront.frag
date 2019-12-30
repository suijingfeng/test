precision highp float;

///
//  Uniforms
//
uniform float transparency_bias;
uniform float transparency_scale;

varying vec2 tex_coord;

///
//  Samplers
//
uniform sampler2D base;

void main(void)
{
   vec4 color = texture2D(base, tex_coord);

   color.a *= transparency_scale;
   color.a += transparency_bias;

   gl_FragColor = color;
//   gl_FragColor = vec4(1.0f, 1.0f, 0.0f, 0.0f);

}
