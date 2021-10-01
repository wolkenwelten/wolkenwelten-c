uniform mat4 matMVP;
uniform vec4 inColor;

in vec4 pos;
in vec2 tex;
in float lval;

out vec2 multiTexCoord;
out vec4 color;

void main(){
	gl_Position   = matMVP * pos;
	multiTexCoord = tex;
	color         = vec4(inColor.rgb * vec3(lval,lval,lval), inColor.a);
}
