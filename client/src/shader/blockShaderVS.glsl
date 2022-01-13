uniform mat4 matMVP;
uniform vec3 transPos;
uniform vec3 sideTints[6];

in uint packedData;

out vec3 texCoord;
out vec3 lightness;

void main(){
  uvec4 pos = uvec4(packedData & 0x1Fu, (packedData >> 5) & 0x1Fu, (packedData >> 10) & 0x1Fu, 1);
  uvec2 taxis[3] = uvec2[3](pos.xy, pos.xz, pos.zy);
  uint flag = (packedData >> 24) & 0x7u;
  vec3 tex = vec3(uvec3(taxis[flag >> 1], (packedData >> 16) & 0xFFu));

  gl_Position = matMVP * (vec4(pos) + vec4(transPos,0.0));
  texCoord    = tex;
  lightness   = sideTints[flag];
}
