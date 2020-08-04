uniform mat4 matMVP;

attribute vec4 pos;
attribute vec4 color;

varying vec4 frontColor;

void main(){
	gl_Position  = matMVP * pos;
	gl_PointSize = 64.0/(gl_Position.z);
	frontColor   = color;
}
