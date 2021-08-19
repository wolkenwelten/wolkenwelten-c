uniform mat4 matMVP;
uniform vec3 transPos;
uniform vec3 sideTint;

in vec4  pos;
in vec3  tex;

out vec3 texCoord;
out vec3 lightness;

void main(){
	gl_Position = matMVP * (pos + vec4(transPos,0.0));
	texCoord    = tex;
	lightness   = sideTint;
}
