uniform mat4 matMVP;
uniform float sizeMul;

in vec4  pos;
in vec4  color;
in float size;

out vec4 frontColor;

void main(){
	gl_Position  = matMVP * pos;
	gl_PointSize = size/(gl_Position.z) * sizeMul;
	frontColor   = color;
}
