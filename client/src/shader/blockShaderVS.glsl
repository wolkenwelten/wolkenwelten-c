uniform mat4 matMVP;
uniform vec3 transPos;

in uint packedData;

out vec3 texCoord;
out vec3 lightColor;

void main(){
	uvec4 pos      = uvec4(packedData & 0x1Fu, (packedData >> 5) & 0x1Fu, (packedData >> 10) & 0x1Fu, 1);
	uvec2 taxis[3] = uvec2[3](uvec2(pos.x, 0x1Fu - pos.y), pos.xz, uvec2(pos.z, 0x1Fu - pos.y));
	uint flag      = (packedData >> 24) & 0x7u;
	vec3 tex       = vec3(uvec3(taxis[flag >> 1], (packedData >> 16) & 0xFFu)) / vec3(2.0,2.0,1.0);
	float lightRaw = float(packedData >> 28) * (1.0 / 16.0);
	float light    = lightRaw * lightRaw;

	lightColor  = vec3(light, light, light);
	gl_Position = matMVP * (vec4(pos) + vec4(transPos,0.0));
	texCoord    = tex;
}
