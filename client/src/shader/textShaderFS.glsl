uniform sampler2D curTex;

in vec2 multiTexCoord;
in vec4 frontColor;

out vec4 fragColor;

void main() {
	vec4 pixel;
	pixel = texture(curTex, multiTexCoord);
	fragColor = frontColor * pixel;
}
