layout(location = 0) in vec3 aPos;
uniform mat4 matMVP;
out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    vec4 pos = matMVP * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}