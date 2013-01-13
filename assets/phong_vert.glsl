#version 110

uniform vec3 lightPosition;
uniform float attenuationFactor;

varying vec3 v;
varying vec3 N;
varying float att;

void main()
{
	v = vec3(gl_ModelViewMatrix * gl_Vertex);       
	N = normalize(gl_NormalMatrix * gl_Normal);

	gl_TexCoord[0] = gl_MultiTexCoord0;

	float distance = length(lightPosition - v);
	float distanceSquared = distance * distance;

	att = 1.0 / (distanceSquared*attenuationFactor);
	//att = 1.0 / (1.0 + 0.0014 * distance + 0.000007 * distance * distance);

	gl_Position = ftransform();
}
