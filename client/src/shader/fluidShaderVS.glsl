uniform mat4 matMVP;
uniform vec3 transPos;

in uvec3 pos;
in uint tex;
in uint flags;

out vec3 texCoord;
out vec3 lightColor;

void main(){
	uvec2 taxis[3] = uvec2[3](pos.xy, pos.xz, pos.zy);
        uint flag      = flags & 0x7;
	vec3 tex       = vec3(uvec3(taxis[flag >> 1], tex)) / vec3(2.0,2.0,1.0);
	float light    = float(flags >> 3) / 32.0;

	lightColor  = vec3(light, light, light);
	gl_Position = matMVP * (vec4(pos) + vec4(transPos,0.0));
	texCoord    = tex;
}
