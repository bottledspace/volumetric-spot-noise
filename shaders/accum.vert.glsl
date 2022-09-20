#version 300 es
precision highp float;
precision highp int;

layout (location = 0) in vec3 loc_in;

float M_PI = 3.1415;

out mat2 Q;
out vec4 U;
out vec4 color;
out float scalefactor;
out vec2 axis;
out float light;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 proj;
uniform sampler2D impulses;
uniform sampler2D impulses2;
uniform float time;

mat3 rotate(vec3 v, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float x = v.x;
    float y = v.y;
    float z = v.z;

    return transpose(mat3(
                x*x*(1.0-c)+c     ,x*y*(1.0-c)-z*s   ,x*z*(1.0-c)+y*s ,
                y*x*(1.0-c)+z*s   ,y*y*(1.0-c)+c     ,y*z*(1.0-c)-x*s ,
                z*x*(1.0-c)-y*s   ,z*y*(1.0-c)+x*s   ,z*z*(1.0-c)+c   ));
}

void main() {
    int id = gl_InstanceID*6;

    mat3 R1;
    vec3 M1 = texelFetch(impulses, ivec2(id%4046,id/4046), 0).xyz;
    R1[0] = texelFetch(impulses, ivec2((id+1)%4046,(id+1)/4046), 0).xyz;
    R1[1] = texelFetch(impulses, ivec2((id+2)%4046,(id+2)/4046), 0).xyz;
    R1[2] = texelFetch(impulses, ivec2((id+3)%4046,(id+3)/4046), 0).xyz;
    vec3 S1 = texelFetch(impulses, ivec2((id+4)%4046,(id+4)/4046), 0).xyz;
    vec4 color1 = texelFetch(impulses, ivec2((id+5)%4046,(id+5)/4046), 0);

    mat3 R2;
    vec3 M2 = texelFetch(impulses2, ivec2(id%4046,id/4046), 0).xyz;
    R2[0] = texelFetch(impulses2, ivec2((id+1)%4046,(id+1)/4046), 0).xyz;
    R2[1] = texelFetch(impulses2, ivec2((id+2)%4046,(id+2)/4046), 0).xyz;
    R2[2] = texelFetch(impulses2, ivec2((id+3)%4046,(id+3)/4046), 0).xyz;
    vec3 S2 = texelFetch(impulses2, ivec2((id+4)%4046,(id+4)/4046), 0).xyz;
    vec4 color2 = texelFetch(impulses2, ivec2((id+5)%4046,(id+5)/4046), 0);

    float x = 0.0;// 0.5+0.5*sin((sin(M1.x+M2.x)*cos(M1.z+M2.z)+time)*5.0);
    float dx = 0.5*cos((M1.x-M1.y-M1.z-0.5*cos(time)-time)*5.0);

    

    vec3 M = M1; //mix(M1,M2,x);
    mat3 R;
    R[0] = mix(R1[0],R2[0],x);
    R[1] = mix(R1[1],R2[1],x);
    R[2] = mix(R1[2],R2[2],x);
    
    R = R*rotate(R[1], dx);
    vec3 S = mix(S1,S2,x);
    color = color1; //mix(color1,color2,x);

    light = 0.5+max(0.0,dot(R1[0],vec3(-0.5,1,-0.5)));

    mat3 Sigma = R*mat3(S.x*S.x,0.0,0.0,
                        0.0,S.y*S.y,0.0,
                        0.0,0.0,S.z*S.z)*transpose(R);

    // Compute camera coords
    U = mvp * vec4(M, 1.0);

    // Compute Jacobian
    float l = length(U.xyz);
    mat3 J = transpose(mat3(
        1.0/U.z,     0.0,  -U.x/(U.z*U.z),
        0.0,     1.0/U.z,  -U.y/(U.z*U.z),
        U.x/l,   U.y/l,   U.z/l));
    mat3 W = mat3(mvp);
    
    
    // Compute variance matrix
    mat3 V = J*W*Sigma*transpose(W)*transpose(J);

    // Setup resampling filter rho
    Q = inverse(mat2(V)+mat2(1.0));
    scalefactor = 1.0/sqrt(2.0*M_PI*determinant(Q));

    vec4 temp = mvp*vec4(R[0],0.0);
    axis = normalize(temp.xy / temp.w);

    // Project U to screen coords
    U /= U.w;
    gl_Position = vec4(0.04*loc_in.xy+U.xy,U.z,1.0);
}
