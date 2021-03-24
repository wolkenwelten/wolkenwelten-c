uniform mat4 matMVP;

in vec4 pos;
in vec2 tex;
in vec4 color;

out vec2 multiTexCoord;
out vec4 frontColor;

void main(){
	gl_Position   = matMVP * pos;
	multiTexCoord = tex * 0.0078125; // 1/128
	frontColor    = color;
}
