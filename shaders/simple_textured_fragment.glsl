#version 400

in vec2 pass_texCoords;

out vec4 frag_colour;

uniform sampler2D textureSampler;

void main() {
  vec4 texel = texture(textureSampler, vec2(pass_texCoords.x, -pass_texCoords.y));

frag_colour = vec4(1.0, 1.0, 1.0, 1.0);
//   frag_colour = vec4(1.0, 1.0, texel.x, 1.0);
  // frag_colour = vec4(normal, 1.0);
//   frag_colour = vec4(0.0, pass_texCoords.x,, 1.0);
}