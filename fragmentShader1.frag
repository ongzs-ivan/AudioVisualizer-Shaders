precision mediump float;
varying vec4 fColor;
varying vec2 fTexCoord;

uniform sampler2D sampler2d;
uniform float Factor1;
uniform float Low;
uniform float Mids[4];
uniform float High;
uniform int HighCount;
uniform float Time;

float thickness = 2.5;
float grad_thickness = 10.0;
float wave_y = 100.0;

float circle_rad = 225.0;
vec2 circle_center = vec2(400.0,300.0);
vec4 circle_color = vec4(0.15, 0.76, 0.65, 1.0);

vec4 redColor = vec4(0.7, 0.0, 0.0, 1.0);
vec4 ntdPurple = vec4(0.89, 0.1, 0.78, 1.0);

float wave1 = 0.0;
float wave2 = 0.0;
float wave3 = 0.0;
float wave4 = 0.0;

float Lerp(float colorA, float colorB, float time)
{
	float color = 0.0;
	color = colorA + time * (colorB - colorA);
	return color;
}

vec4 ColorLerp(vec4 colorA, vec4 colorB, float time)
{
	float finalR = 0.0;
	float finalG = 0.0;
	float finalB = 0.0;
	
	finalR = colorA.r + time * (colorB.r - colorA.r);
	finalG = colorA.g + time * (colorB.g - colorA.g);
	finalB = colorA.b + time * (colorB.b - colorA.b);
	
	return vec4(finalR, finalG, finalB, 1.0);
}

void main()
{
	vec4 tempColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 finalColor;

	tempColor = texture2D(sampler2d, fTexCoord);
	
	// if (gl_FragCoord.x < 200.0)
	// {
		// wave = 300.0 + sin(gl_FragCoord.x * frequency + Factor1 * speed) * Mids[0] * waveAmp;
	// }
	
	if (Low >= 0.25)
	{
		if (tempColor.r > 0.8 && tempColor.g > 0.8 && tempColor.b > 0.8)
		{
			tempColor = ColorLerp(tempColor, (tempColor + Low * 0.2 * 1.1), Time * 0.05);
		}
	}
	
	if (High >= 0.25)
	{
		if (tempColor.r > 0.9 && tempColor.g < 0.65 && tempColor.b < 0.65)
		{
			tempColor = ColorLerp(tempColor, (tempColor + High * 0.2 * 0.25), Time * 0.05);
		}
	}

	// Mid Waves
	wave1 = wave_y + sin(gl_FragCoord.x * 0.2 + Factor1 * 1.0 * abs(High)) * Mids[0] * 5.0;
	wave2 = wave_y + sin(gl_FragCoord.x * 0.1 + Factor1 * 2.0 * abs(High)) * Mids[1] * 7.5;
	wave3 = wave_y + sin(gl_FragCoord.x * 0.01 + Factor1 * 5.0 * abs(High)) * Mids[2] * 5.0;
	wave4 = wave_y + sin(gl_FragCoord.x * 0.05 + Factor1 * 10.0 * abs(High)) * Mids[3] * 2.5;
		
	if ((gl_FragCoord.y > (wave1 - thickness)) && (gl_FragCoord.y < (wave1 + thickness)))
	{
		tempColor = tempColor + redColor;
	}
	if ((gl_FragCoord.y > (wave2 - thickness)) && (gl_FragCoord.y < (wave2 + thickness)))
	{
		tempColor = tempColor + redColor;
	}
	if ((gl_FragCoord.y > (wave3 - thickness)) && (gl_FragCoord.y < (wave3 + thickness)))
	{
		tempColor = tempColor + redColor;
	}
	if ((gl_FragCoord.y > (wave4 - thickness)) && (gl_FragCoord.y < (wave4 + thickness)))
	{
		tempColor = tempColor + redColor;
	}
		
	// Gradient Wave
	float wave_centre = sin(gl_FragCoord.x * 0.05 + Factor1 * 5.0) * 2.5 + wave_y;
	float gradient = 1.0 - abs(gl_FragCoord.y - wave_centre)/grad_thickness;
	float gradient2 = abs(gl_FragCoord.y - wave_centre)/grad_thickness;
	vec4 gradColor = ntdPurple;
	if (gl_FragCoord.y > (wave_centre - grad_thickness) && gl_FragCoord.y < (wave_centre + grad_thickness))
	{
		tempColor = tempColor + (gradColor * gradient) * (gradColor * gradient2);
	}
	
	circle_rad = circle_rad + cos(Low) * 2.0 * Mids[1];
	
	float current_dist = distance(circle_center, gl_FragCoord.xy);
	float grad1 = (current_dist - circle_rad)/circle_rad;
	
	if (current_dist > circle_rad )
	{
		circle_color = vec4
		(0.0,
		grad1,
		grad1 * 0.5,
		1.0);
		tempColor = tempColor + (circle_color * grad1);
	}
	
	tempColor.a = 1.0;
	gl_FragColor = tempColor;
}