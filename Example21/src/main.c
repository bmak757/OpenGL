/**
 * OpenGL 4 - Example 21
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

/**
 * The used shader program.
 */
static GLUSshaderprogram g_program;

/**
 * Location of the model view projection matrix in the shader program.
 */
static GLint g_modelViewProjectionMatrixLocation;

/**
 * The location of the vertex in the shader program.
 */
static GLint g_vertexLocation;

/**
 * The location of the texture coordinate in the shader program.
 */
static GLint g_texCoordLocation;

/**
 * The location of the texture in the shader program.
 */
static GLint g_textureLocation;

/**
 * The VBO for the vertices.
 */
static GLuint g_verticesVBO;

/**
 * The VBO for the texture coordinates.
 */
static GLuint g_texCoordsVBO;

/**
 * The VBO for the indices.
 */
static GLuint g_indicesVBO;

/**
 * The VAO for the vertices etc.
 */
static GLuint g_vao;

/**
 * The texture.
 */
static GLuint g_texture;

/**
 * The number of the indices
 */
static GLuint g_numberIndicesPlane;

/**
 * Width of the image.
 */
static GLuint g_imageWidth = 640;

/**
 * Height of the image.
 */
static GLuint g_imageHeight = 480;

static GLuint g_localSize = 16;

/**
 * The used compute shader program.
 */
static GLUSshaderprogram g_computeProgram;

/**
 * The location of the texture in the compute shader program.
 */
static GLint g_computeTextureLocation;

GLUSboolean init(GLUSvoid)
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;
    GLUStextfile computeSource;

    GLUSshape plane;

    glusLoadTextFile("../Example21/shader/texture.vert.glsl", &vertexSource);
    glusLoadTextFile("../Example21/shader/texture.frag.glsl", &fragmentSource);

    glusBuildProgramFromSource(&g_program, (const GLchar**) &vertexSource.text, 0, 0, 0, (const GLchar**) &fragmentSource.text);

    glusDestroyTextFile(&vertexSource);
    glusDestroyTextFile(&fragmentSource);


    glusLoadTextFile("../Example21/shader/texture.comp.glsl", &computeSource);

    glusBuildComputeProgramFromSource(&g_computeProgram, (const GLchar**) &computeSource.text);

    glusDestroyTextFile(&computeSource);

    //

    // Retrieve the uniform locations in the program.
    g_modelViewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewProjectionMatrix");
    g_textureLocation = glGetUniformLocation(g_program.program, "u_texture");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");


    g_computeTextureLocation = glGetUniformLocation(g_computeProgram.program, "u_texture");

    //

    // Generate and bind a texture.
    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);

    // Create an empty image.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, g_imageWidth, g_imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Setting the texture parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    //

    // Use a helper function to create a rectangular plane.
    glusCreateRectangularPlanef(&plane, (GLfloat) g_imageWidth / 2.0f, (GLfloat) g_imageHeight / 2.0f);

    // Store the number indices, as we will render with glDrawElements.
    g_numberIndicesPlane = plane.numberIndices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) plane.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_texCoordsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 2 * sizeof(GLfloat), (GLfloat*) plane.texCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Generate a VBO for the indices.
    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane.numberIndices * sizeof(GLuint), (GLuint*) plane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Now we can destroy the shape, as all data is now on the GPU.
    glusDestroyShapef(&plane);

    //

    glUseProgram(g_program.program);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_texCoordLocation);

    // Also bind the indices to the VAO.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    //

    // Also bind created texture ...
    glBindTexture(GL_TEXTURE_2D, g_texture);

    // ... and bind this texture as an image, as we will write to it.
    glBindImageTexture(0, g_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

    // ... and as this is texture number 0, bind the uniform to the program.
    glUniform1i(g_textureLocation, 0);

    //

    glUseProgram(g_computeProgram.program);

    // Pass texture number 0 to the compute shader as well.
    glUniform1i(g_computeTextureLocation, 0);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat viewMatrix[16];
    GLfloat modelViewProjectionMatrix[16];

    glViewport(0, 0, width, height);

    // Create the view matrix.
    glusLookAtf(viewMatrix, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Create a orthogonal projection matrix.
    glusOrthof(modelViewProjectionMatrix, -(GLfloat) width / 2.0f, (GLfloat) width / 2.0f, -(GLfloat) height / 2.0f, (GLfloat) height / 2.0f, 1.0f, 100.0f);

    // MVP = P * V * M (Note: Here we do not have model matrix).
    glusMatrix4x4Multiplyf(modelViewProjectionMatrix, modelViewProjectionMatrix, viewMatrix);

    // Use render program.
    glUseProgram(g_program.program);

    // Pass the model view projection matrix to the current active program.
    glUniformMatrix4fv(g_modelViewProjectionMatrixLocation, 1, GL_FALSE, modelViewProjectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Switch to the compute shader.
    glUseProgram(g_computeProgram.program);

    // Create threads depending on width, height and block size. In this case we have 1200 threads.
    glDispatchCompute(g_imageWidth / g_localSize, g_imageHeight / g_localSize, 1);

    // Switch back to the render program.
    glUseProgram(g_program.program);

    // Here we draw the plane / rectangle using the indices, stored in the VBO.
    glDrawElements(GL_TRIANGLES, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (g_verticesVBO)
    {
        glDeleteBuffers(1, &g_verticesVBO);

        g_verticesVBO = 0;
    }

    if (g_texCoordsVBO)
    {
        glDeleteBuffers(1, &g_texCoordsVBO);

        g_texCoordsVBO = 0;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (g_indicesVBO)
    {
        glDeleteBuffers(1, &g_indicesVBO);

        g_indicesVBO = 0;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_texture)
    {
        glDeleteTextures(1, &g_texture);

        g_texture = 0;
    }

    glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);

        g_vao = 0;
    }

    glUseProgram(0);

    glusDestroyProgram(&g_program);

    glusDestroyProgram(&g_computeProgram);
}

int main(int argc, char* argv[])
{
    glusInitFunc(init);

    glusReshapeFunc(reshape);

    glusUpdateFunc(update);

    glusTerminateFunc(terminate);

    glusPrepareContext(4, 3, GLUS_FORWARD_COMPATIBLE_BIT);

    if (!glusCreateWindow("GLUS Example Window", 640, 480, 0, 0, GLUS_FALSE))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusRun();

    return 0;
}
