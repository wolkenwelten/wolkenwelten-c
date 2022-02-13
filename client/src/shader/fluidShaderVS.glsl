uniform mat4 matMVP;
uniform vec3 transPos;

in uvec3 pos;
in uint tex;
in uint flag;

out vec3 texCoord;
out vec3 lightColor;

void main(){
	uvec2 taxis[3] = uvec2[3](pos.xy, pos.xz, pos.zy);
        uint side     = flag & 0x7u;
	vec3 tex       = vec3(uvec3(taxis[side >> 1], tex)) / vec3(2.0,2.0,1.0);
	float light    = float(flag >> 3) / 32.0;

	lightColor  = vec3(light, light, light);
	gl_Position = matMVP * (vec4(pos, 1.0) + vec4(transPos, 0.0));
	texCoord    = tex;
}
