uniform mat4 matMVP;

in vec4 pos;
in vec4 color;

out vec4 frontColor;

void main(){
	gl_Position  = matMVP * pos;
	gl_PointSize = 2048.0/(gl_Position.z);
	frontColor   = color;
}
