#version 120

#extension GL_ARB_texture_rectangle : enable

uniform sampler2D leftTextureY;
uniform sampler2D leftTextureU;
uniform sampler2D leftTextureV;
//uniform sampler2D rightTextureY;
//uniform sampler2D rightTextureU;
//uniform sampler2D rightTextureV;

uniform vec2 ImageAngleSize;

uniform vec2 roffset;
uniform vec2 loffset;

uniform int mode;

varying vec3 norm;

//const vec3 vr = vec3( 1.0000,  0.0000,  1.13983 );
//const vec3 vg = vec3( 1.0000, -0.39465, -0.58060 );
//const vec3 vb = vec3( 1.0000,  2.03211,  0.0000 );

vec2 getLookup(vec3 pos)
{
	//float theta = atan( (pos.y), (pos.x) );
	// float phi   = atan( sqrt( (pos.x*pos.x) + (pos.y*pos.y)), (pos.z) );

	float x = pos.x;
	float y = pos.z;
	float z = pos.y;
	/*float x = pos.x;
	float y = pos.y;
	float z = pos.z;*/
	
	vec2 p =  vec2(((atan(y, x))/ 3.1415926 + 1.0) * 0.5, (asin(z) / 3.1415926 + 0.5));

	/*
	float sx = 360.0 / ImageAngleSize.x;
	float sy = 180.0 / ImageAngleSize.y;
	*/
	
	float sx = 0.9575;
	float sy = 1.0;
	
	
	p.x=(p.x-0.5)*sx + 0.5;
	p.y=(p.y-0.5)*sy + 0.5; 
	p.y = 1.f-p.y;
	//now we need to normalize
	return p;
}


vec3 decodeYUV(vec2 texcoord)
{
	//do the yuv decompression here..
	//start with just one texture...
	float lY = texture2D(leftTextureY, texcoord).x;
	//vec2 lTC = vec2(texcoord.x, texcoord.y);
	float lU = texture2D(leftTextureU, texcoord).x;
	float lV = texture2D(leftTextureV, texcoord).x;
	
	lY = (lY - 0.0625) * 1.1643;
	lU = lU - 0.5;
	lV = lV - 0.5;
	
	texcoord.x = lU * 0.39173;
	texcoord.y = lV * 0.81290;
	
	vec3 rgbVec = vec3(lY + (1.5958 * lV), lY - texcoord.x - texcoord.y, (lU * 2.017) + lY);
	//rgbVec.r = clamp(rgbVec.r, 0.0, 1.0);
	//rgbVec.g = clamp(rgbVec.g, 0.0, 1.0);
	//rgbVec.b = clamp(rgbVec.b, 0.0, 1.0);
	rgbVec = clamp(rgbVec, vec3(0.0), vec3(1.0));
	
	return rgbVec;
}

void main ()
{
	vec2 p = getLookup(normalize(norm));
	
	//p = normalize(p);
	//lets make it black if it is outside of the bounds
	if ((p.x < 0.0 && p.y < 0.0) || (p.x > 1.0 && p.y > 1.0))
	{
		 gl_FragColor = vec4(1.0,0.0,0.0,1.0);
	}
	else
	{
	
		vec3 rgbVec = decodeYUV(p);

		
		//vec3 r = vec3(texture2D(rightTexture, vec2(p) + roffset));
		//anaglyph
		if(mode == 0)
		{
			gl_FragColor = vec4(rgbVec,1.0);
		}
		else
		{
			//we are coming into this clause right now..
			// (mode == 1)
			gl_FragColor = vec4(rgbVec,1.0);
		}
		//else
		//	gl_FragColor = vec4(rgbVec.xyz,1.0);
	}
	
	
	//gl_FragColor = vec4(p, 0.0, 1.0);
	
}

void mainnew()
{

	vec3 n = normalize(norm);
	//vec3 n = normalize(norm.zxy);
	vec3 color = vec3(n);

	
	float m = 2.0 * sqrt(n.x*n.x + n.y*n.y + (n.z+1.0)*(n.z+1.0));
	
	vec2 texcoord = n.xy / m + vec2(0.5);	

	//color = decodeYUV(texcoord);
	
	color = vec3(1,0,1);
	
	gl_FragColor = vec4(color, 1.0);
}


