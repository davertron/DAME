#version 110

uniform float quantize;
uniform vec3 lightPosition;
uniform float quantLevels;
uniform vec4 drawColor;
uniform float colorInside;

varying vec3 v;
varying vec3 N;
varying float att;

void main()
{
    const vec4  ambient = vec4(0.1, 0.1, 0.1, 1);
    const vec4  diffuse = vec4(0.9, 0.9, 0.9, 1);

    vec3 L = normalize(lightPosition - v);
    vec3 E = normalize(-v);
    vec3 NN;
    if(colorInside > 0.0){
        NN = -N;
    } else {
        NN = N;
    }

    // ambient term
    vec4 Iamb = ambient;

    // diffuse term
    vec4 Idiff = vec4(1.0, 1.0, 1.0, 1.0) * diffuse;
    float angle = dot(NN,L);
    Idiff *= max(angle, 0.0);
    Idiff = clamp(Idiff, 0.0, 1.0);

    vec4 color = (Iamb + Idiff) * drawColor * att;

    if(quantize > 0.0){
        // store previous alpha value
        float alpha = color.a;
        // quantize process: multiply by factor, round and divde by factor
        color = floor(0.5 + (quantLevels * color)) / quantLevels;
        // set fragment/pixel color
        color.a = alpha;
    }

    gl_FragColor = color;
}
