uniform mat4 matMVP;

in vec4 pos;
in vec2 tex;

out vec2 multiTexCoord;

void main(){
	gl_Position   = matMVP * pos;
	multiTexCoord = tex * 0.0078125; // 1/128
}
