varying vec4 destinationColor;
varying vec2 textureColor;

uniform float time;

void main(){
    vec4 color = vec4(time, 0.7 ,0.6 , 1.0);
    gl_FragColor = color;
}