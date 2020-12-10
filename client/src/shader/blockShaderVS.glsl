uniform mat4 matMVP;
uniform vec3 transPos;
uniform float colorBrightness;

in vec4  pos;
in vec3  tex;
in float flag;

out vec3 texCoord;
out float lightness;

void main(){
	gl_Position = matMVP * (pos + vec4(transPos,0.0));
	texCoord    = tex;
	lightness   = (0.5 + (fract(flag / 4.0) / 2.0)) * colorBrightness;
}
