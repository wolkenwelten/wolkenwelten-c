uniform sampler2D curTex;

in vec2 multiTexCoord;
in vec4 frontColor;

out vec4 fragColor;

void main() {
	fragColor = frontColor * texture(curTex, multiTexCoord);
}
