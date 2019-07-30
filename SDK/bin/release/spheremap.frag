#version 120

#extension GL_ARB_texture_rectangle : enable

uniform sampler2D leftTextureY;
uniform sampler2D leftTextureU;
uniform sampler2D leftTextureV;
uniform sampler2D rightTextureY;
uniform sampler2D rightTextureU;
uniform sampler2D rightTextureV;

uniform vec2 ImageAngleSize;

uniform vec2 roffset;
uniform vec2 loffset;

uniform int mode;

varying vec3 norm;
varying vec2 texcoord;

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

	vec2 p =  vec2(((atan(y, x))/ 3.1415926 + 1.0) * 0.5, (asin(z) / 3.1415926 + 0.5));

	/*
	float sx = 360.0 / ImageAngleSize.x;
	float sy = 180.0 / ImageAngleSize.y;
	*/
	
	float sx = 1.0f;//0.96;
	float sy = 1.0;
	
	
	p.x=(p.x-0.5)*sx + 0.5;
	p.y=(p.y-0.5)*sy + 0.5; 
	p.y = 1.f-p.y;
	//now we need to normalize
	return p;
}


vec3 decodeYUVLeft(vec2 texcoord)
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

vec3 decodeYUVRight(vec2 texcoord)
{
	//do the yuv decompression here..
	//start with just one texture...
	float lY = texture2D(rightTextureY, texcoord).x;
	//vec2 lTC = vec2(texcoord.x, texcoord.y);
	float lU = texture2D(rightTextureU, texcoord).x;
	float lV = texture2D(rightTextureV, texcoord).x;
	
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

void mainold ()
{
	vec2 p = getLookup(normalize(norm));
	
	//p = normalize(p);
	//lets make it black if it is outside of the bounds
	if ((p.x < 0.1 && p.y < 0.1) || (p.x > 0.9 && p.y > 0.9))
	{
		 gl_FragColor = vec4(1.0,0.0,0.0,1.0);
	}
	else
	{
	
		vec3 rgbVec = decodeYUVLeft(p);

		
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

void main()
{
	
	

	// TODO: multiply this by the model matrix!
	vec3 r = normalize(norm);
	
	
	// rotate to match CAVE
	
	r.xyz = r.zyx;
	r.x *= -1.0;
	
	float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z+1)*(r.z+1));
				 
	// offset angle to straighten videos
	// for motorcycle
	//const float rotationAngle = 200.0;
	
	// for uw footie
	//const float rotationAngle = -90.0;

	// for dual 360
	const float rotationAngle = 180.0;
	
	// for 4k motorcycle
	//const float rotationAngle = 170.0;

	
	
	// rotate 2D texcoord
	vec2 tex = r.xy; 
	float a = radians(rotationAngle);
	float cs = cos(a);
	float sn = sin(a);

	vec2 uv = vec2(tex.x * cs - tex.y * sn, tex.x * sn + tex.y*cs);
		

	/*
	// correct but looks weird


	// scale back from 360 to 240 fov (inverse of 240/360) 
	uv *= 1.5;
	
	// scale back to [0..1] tex coords
	uv = uv / m + 0.5;

	
	vec3 color = vec3(0.0);
	if (dot(uv, uv) <= 1.0)
		color = decodeYUV(uv);
	*/
	
	
	// incorrect, but looks better (except in the edge case) 
	// scale back to [0..1]
	uv = uv / m + 0.5;
	
	vec3 color;
	
	if (mode == 2)
	{
		vec3 colorRight = decodeYUVRight(uv);
		vec3 colorLeft = decodeYUVLeft(uv);

		color = vec3(colorRight.r, colorLeft.g, colorLeft.b);
	}
	else if (mode == 1)
	{
		color = decodeYUVRight(uv);
	}
	else
	{
		color = decodeYUVLeft(uv);
	}
	
	// block out 120 arc of 360fly camera
	if (r.z < 0) {
	
		if (dot(r.xy, r.xy) < 0.3) {
			color = vec3(0.0); //0, 0.0, 0.0);
		
			// at this point we should rescale the uv coordinates
		}
	
	}
	
	
	gl_FragColor = vec4(color, 1.0);
}


