uniform mat4 matMVP;

in vec4 pos;
in vec2 tex;
in float lval;

out vec2 multiTexCoord;
out float lightness;

void main(){
	gl_Position   = matMVP * pos;
	multiTexCoord = tex;
	lightness     = lval;
}
