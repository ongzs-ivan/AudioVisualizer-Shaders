attribute vec4 vPosition;
attribute vec4 vColor;
attribute vec2 vTexCoord;

varying vec2 fTexCoord;
varying vec4 fColor;

void main()
{
	fColor = vColor;
	fTexCoord = vTexCoord;
	
	gl_Position = vPosition;
}