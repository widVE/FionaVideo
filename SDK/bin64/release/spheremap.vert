#version 120

varying vec3 norm;

void main()
{
	 gl_TexCoord[0] = gl_MultiTexCoord0;
	 gl_TexCoord[1] = gl_MultiTexCoord1;
	 gl_TexCoord[2] = gl_MultiTexCoord2;
	 gl_TexCoord[3] = gl_MultiTexCoord3;
	 norm = normalize( gl_Normal );
	 
	 
	 
	 gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}


/*
void main()
{

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_TexCoord[2] = gl_MultiTexCoord2;
	gl_TexCoord[3] = gl_MultiTexCoord3;
	norm = normalize( ( gl_Normal ) );

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;


	// TODO: multiply this by the model matrix!
	vec4 p = gl_Vertex;	
	vec3 r = normalize(p.xyz);
	
	float m = 2.0 * sqrt(r.x*r.x + r.y*r.y + (r.z+1)*(r.z+1));
				 
	
	const float rotationAngle = 71;
	

	// rotate 2D texcoord
	vec2 tex = r.xy; 
	float a = radians(rotationAngle);
	float cs = cos(a);
	float sn = sin(a);

	vec2 uv = float2(tex.x * cs - tex.y * sn, tex.x * sn + tex.y*cs);
	uv = uv / m + 0.5;
	texcoord = uv;

}

*/

