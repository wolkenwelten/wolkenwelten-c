uniform mat4 matMVP;
uniform float colorBrightness;
uniform float colorAlpha;

in vec4 pos;
in vec2 tex;
in float lval;

out vec2 multiTexCoord;
out float lightness;
out float alpha;

void main(){
	gl_Position   = matMVP * pos;
	multiTexCoord = tex;
	lightness     = lval * colorBrightness;
	alpha         = colorAlpha;
}
