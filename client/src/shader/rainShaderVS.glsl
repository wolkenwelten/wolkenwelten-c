uniform mat4 matMVP;
uniform float sizeMul;

in vec4 pos;
in vec4 color;

void main(){
	gl_Position  = matMVP * vec4(pos.xyz,1.0);
	gl_PointSize = pos.w / (gl_Position.z) * sizeMul;
}
