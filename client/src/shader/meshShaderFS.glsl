uniform sampler2D curTex;

in vec2 multiTexCoord;
in float lightness;
in float alpha;

out vec4 fragColor;

void main() {
	fragColor = texture(curTex, multiTexCoord) * vec4(lightness,lightness,lightness,alpha);
}
