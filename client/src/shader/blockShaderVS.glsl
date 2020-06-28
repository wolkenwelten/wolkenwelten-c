uniform mat4 matMVP;
uniform vec3 transPos;

attribute vec4 pos;
attribute vec2 tex;
attribute float flag;

varying vec2 multiTexCoord;
varying float lightness;

void main(){
	gl_Position   = matMVP * (pos + vec4(transPos,0.0));
	multiTexCoord = tex;
	lightness = 0.5 + (fract(flag / 4.0) / 2.0);
}
