uniform mat4 matMVP;

attribute vec4  pos;
attribute vec4  color;
attribute float size;

varying vec4 frontColor;

void main(){
	gl_Position  = matMVP * pos;
	gl_PointSize = size/(gl_Position.z);
	frontColor   = color;
}
