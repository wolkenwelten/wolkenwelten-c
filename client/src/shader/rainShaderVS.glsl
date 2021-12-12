uniform mat4 matMVP;
uniform float sizeMul;

in vec4 pos;

void main(){
	gl_Position  = matMVP * vec4(pos.xyz,1.0);
	gl_PointSize = min(32.0,pos.w / (gl_Position.z) * sizeMul);
}
