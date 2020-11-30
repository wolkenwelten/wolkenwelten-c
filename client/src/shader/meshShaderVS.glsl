uniform mat4 matMVP;
uniform float colorBrightness;

attribute vec4 pos;
attribute vec2 tex;
attribute float lval;

varying vec2 multiTexCoord;
varying float lightness;

void main(){
	gl_Position   = matMVP * pos;
	multiTexCoord = tex;
	lightness     = lval * colorBrightness;
}
