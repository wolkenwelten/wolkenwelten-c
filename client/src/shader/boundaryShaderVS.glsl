uniform mat4 matMVP;

in vec4 pos;
in vec4 color;

out vec2 multiTexCoord;
out vec4 frontColor;

void main(){
	gl_Position  = matMVP * pos;
	frontColor   = color;
}
