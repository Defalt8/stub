#version 330 core

precision mediump float;

uniform sampler2D u_Texture0;
uniform bool u_SampleTexture = false;
uniform vec4 u_ObjectColor = vec4(1.0,1.0,1.0,1.0);
uniform float PI = 3.141592653;
uniform vec4 BG = vec4(0.4,0.8,.9,1.);

in vec2 v_TextureCoord;
in vec4 v_Position;
out vec4 o_FragColor;

vec4 blend(vec4 S, vec4 D) { return vec4(S.rgb*S.a+D.rgb*(1-S.a), S.a+D.a*(1-S.a)); }

bool fequals(float a, float b, float epsilon) {
	return abs(a - b) <= epsilon;
}

// comp: 0 - equals, 1 - less-or-equal, 2 - greater-or-equal
vec4 plot(vec2 coord, float slope, vec2 ref_coord, int comp, float thickness, vec4 fg, vec4 bg)
{
	vec4 RC = bg;
	float epsilon = thickness + 0.005 + min(abs(slope), 100.0) * thickness;
	if( (comp == 1 && coord.y <= ref_coord.y) ||
		(comp == 2 && coord.y >= ref_coord.y) ||
		(comp == 0 && fequals(ref_coord.y, coord.y, epsilon/4))
	) {
		RC = fg;
	} else if(fequals(ref_coord.y, coord.y, epsilon)) {
		float epd = (abs(ref_coord.y - coord.y) - epsilon);
		float epf = 2*(pow(epd, 2.0) / pow(epsilon, 2.0));
		RC = blend(vec4(fg.rgb, fg.a*epf), bg);
	}
	return RC;
}

float cuttoffHigh(float x, float treshold) { return x <= treshold ? x : (sign(x) * treshold); }

float F1(float t) { return cuttoffHigh(0.7*(sin(t*PI)*sin(t*PI*2)*sin(t*PI*3)), 0.0); }
float F2(float x) { return 0.9-x*x; }
float F3(float x) { return 0.5*(sin(x*PI*20)) * (cos(x*PI*2)); }

void main() {
	vec4 CBG;
	if(fequals(v_Position.y, 0.0, 0.002)) {
		CBG = blend(vec4(1.0,0.0,0.0,0.25), BG); // x-axis
	} else if(fequals(v_Position.x, 0.0, 0.002)) {
		CBG = blend(vec4(0.0,1.0,0.0,0.25), BG); // y-axis
	} else if(fequals(v_Position.x, -0.5, 0.002) ||
		fequals(v_Position.x, 0.5, 0.002) ||
		fequals(v_Position.y, -0.5, 0.002) ||
		fequals(v_Position.y, 0.5, 0.002)
	) {
		CBG = blend(vec4(0.1,0.1,0.1,0.05), BG);
	} else {
		CBG = BG;
	}
	// useless comment
	float slope_delta = 0.0001;
	vec2 ref_coord = v_Position.xy;
	vec2 coord1 = vec2(ref_coord.x, F1(ref_coord.x)), coord1_1 = vec2(ref_coord.x + slope_delta, F1(ref_coord.x + slope_delta));
	vec2 coord2 = vec2(ref_coord.x, F2(ref_coord.x)), coord2_1 = vec2(ref_coord.x + slope_delta, F2(ref_coord.x + slope_delta));
	vec2 coord3 = vec2(ref_coord.x, F3(ref_coord.x)), coord3_1 = vec2(ref_coord.x + slope_delta, F3(ref_coord.x + slope_delta));
	float slope1 = (coord1_1.y - coord1.y) / slope_delta;
	float slope2 = (coord2_1.y - coord2.y) / slope_delta;
	float slope3 = (coord3_1.y - coord3.y) / slope_delta;
	CBG = blend(plot(coord2, slope2, ref_coord, 1, 0.005, vec4(1.0,0.1,0.3,1.0), CBG), CBG);
	CBG = blend(plot(coord1, slope1, ref_coord, 2, 0.002, vec4(0.6,0.9,0.1,0.8), CBG), CBG);
	CBG = blend(plot(coord3, slope3, ref_coord, 0, 0.004, vec4(0.0,0.0,0.0,0.9), CBG), CBG);
	o_FragColor = CBG;
}