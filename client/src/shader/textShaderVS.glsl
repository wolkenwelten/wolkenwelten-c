uniform mat4 matMVP;

attribute vec4 pos;
attribute vec2 tex;
attribute vec4 color;

varying vec2 multiTexCoord;
varying vec4 frontColor;

void main(){
	gl_Position   = matMVP * pos;
	multiTexCoord = tex / 128.0;
	frontColor    = color;
}
