uniform sampler2D background;
uniform sampler2D foreground;

void main(void){
	vec2 st = gl_TexCoord[0].st;
	float foregroundAlpha = texture2D(foreground, st).a;
	gl_FragColor = mix(texture2D(foreground, st), texture2D(background, st), 1.0-foregroundAlpha);
}
