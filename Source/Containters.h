/*
  ==============================================================================

    Containters.h
    Created: 23 Sep 2019 3:20:50pm
    Author:  Dmytro Kiro

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "util/WavefrontObjParser.h"

struct Vertex {
    float position[3];
    float normal[3];
    float color[4];
    float texCoord[2];
};

struct Attributes {
    std::unique_ptr<OpenGLShaderProgram::Attribute> position, normal, sourceColor, textureCoordIn;

    Attributes(OpenGLContext &openGLContext, OpenGLShaderProgram &shaderProgram) {
        position.reset(createAttribute(openGLContext, shaderProgram, "position"));
        normal.reset(createAttribute(openGLContext, shaderProgram, "normal"));
        sourceColor.reset(createAttribute(openGLContext, shaderProgram, "sourceColour"));
        textureCoordIn.reset(createAttribute(openGLContext, shaderProgram, "textureCoordIn"));
    }

    void enable(OpenGLContext &openGlContext) {
        if (position.get() != nullptr) {
            openGlContext.extensions.glVertexAttribPointer(position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                                           0);
            openGlContext.extensions.glEnableVertexAttribArray(position->attributeID);
        }

        if (normal.get() != nullptr) {
            openGlContext.extensions.glVertexAttribPointer(normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                                           (GLvoid *) (sizeof(float) * 3));
            openGlContext.extensions.glEnableVertexAttribArray(normal->attributeID);
        }

        if (sourceColor.get() != nullptr) {
            openGlContext.extensions.glVertexAttribPointer(sourceColor->attributeID, 4, GL_FLOAT, GL_FALSE,
                                                           sizeof(Vertex), (GLvoid *) (sizeof(float) * 6));
            openGlContext.extensions.glEnableVertexAttribArray(sourceColor->attributeID);
        }
        if (textureCoordIn.get() != nullptr) {
            openGlContext.extensions.glVertexAttribPointer(textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE,
                                                           sizeof(Vertex), (GLvoid *) (sizeof(float) * 10));
            openGlContext.extensions.glEnableVertexAttribArray(textureCoordIn->attributeID);
        }
    }

    void disable(OpenGLContext &openGlContext) {
        if (position.get() != nullptr) openGlContext.extensions.glDisableVertexAttribArray(position->attributeID);
        if (normal.get() != nullptr) openGlContext.extensions.glDisableVertexAttribArray(normal->attributeID);
        if (sourceColor.get() != nullptr) openGlContext.extensions.glDisableVertexAttribArray(sourceColor->attributeID);
        if (textureCoordIn.get() != nullptr)
            openGlContext.extensions.glDisableVertexAttribArray(textureCoordIn->attributeID);
    }

    static OpenGLShaderProgram::Attribute *createAttribute(OpenGLContext &openGlContext,
                                                           OpenGLShaderProgram &shaderProgram,
                                                           const String &attributeName) {
        if (openGlContext.extensions.glGetAttribLocation(shaderProgram.getProgramID(), attributeName.toRawUTF8()) < 0)
            return nullptr;
        return new OpenGLShaderProgram::Attribute(shaderProgram, attributeName.toRawUTF8());
    }
};

struct Uniforms {
    std::unique_ptr<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;

    Uniforms(OpenGLContext &openGLContext, OpenGLShaderProgram &shaderProgram) {
        projectionMatrix.reset(createUniform(openGLContext, shaderProgram, "projectionMatrix"));
        viewMatrix.reset(createUniform(openGLContext, shaderProgram, "viewMatrix"));
    }

    static OpenGLShaderProgram::Uniform * createUniform(OpenGLContext &openGlContext, OpenGLShaderProgram &shaderProgram, const String &uniformName) {
        if (openGlContext.extensions.glGetUniformLocation(shaderProgram.getProgramID(), uniformName.toRawUTF8()) < 0)
            return nullptr;

        return new OpenGLShaderProgram::Uniform(shaderProgram, uniformName.toRawUTF8());
    }
};

struct Shape {
    Shape(OpenGLContext& openGlContext){
        auto dir = File::getCurrentWorkingDirectory();
        int numTries = 0;
        while (!dir.getChildFile("Resources").exists() && numTries++ < 15){
            dir = dir.getParentDirectory();
        }
        if(shapeFile.load(dir.getChildFile("Resources").getChildFile("supastarOBJ.obj")).wasOk()){
            for(auto* shape: shapeFile.shapes){
                vertexBuffers.add(new VertexBuffer(openGlContext, *shape));
            }
        }
    }

    void draw(OpenGLContext& glContext, Attributes& glAttributes){
        for(auto* vertexBuffer: vertexBuffers){
            vertexBuffer->bind();

            glAttributes.enable(glContext);
            glDrawElements(GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, 0);
            glAttributes.disable(glContext);
        }
    }

    struct VertexBuffer {
        GLuint vertexBuffer, indexBuffer;
        int numIndices;
        OpenGLContext &openGlContext;

        VertexBuffer(OpenGLContext &context, WavefrontObjFile::Shape &shape) : openGlContext(context) {
            numIndices = shape.mesh.indices.size();
            openGlContext.extensions.glGenBuffers(1, &vertexBuffer);
            openGlContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

            Array<Vertex> vertices;
            createVertexListFromMesh(shape.mesh, vertices, Colours::green);

            openGlContext.extensions.glBufferData(GL_ARRAY_BUFFER,
                                                  static_cast<GLsizeiptr>(static_cast<size_t>(vertices.size()) *
                                                                          sizeof(Vertex)), vertices.getRawDataPointer(),
                                                  GL_STATIC_DRAW);
            openGlContext.extensions.glGenBuffers(1, &indexBuffer);
            openGlContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            openGlContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                                  static_cast<GLsizeiptr>(static_cast<size_t>(numIndices) *
                                                                          sizeof(juce::uint32)),
                                                  shape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
        }

        ~VertexBuffer(){
            openGlContext.extensions.glDeleteBuffers(1, &vertexBuffer);
            openGlContext.extensions.glDeleteBuffers(1, &indexBuffer);
        }

        void bind(){
            openGlContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            openGlContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        }

        static void createVertexListFromMesh(const WavefrontObjFile::Mesh &mesh, Array<Vertex> &list, Colour colour) {
            auto scale = 0.2f;
            WavefrontObjFile::TextureCoord defaultTextureCoord{0.5f, 0.5f};
            WavefrontObjFile::Vertex defaultNormal{0.5f, 0.5f};

            for (auto i = 0; i < mesh.vertices.size(); ++i)                  // [7]
            {
                const auto& v = mesh.vertices.getReference (i);
                const auto& n = i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;
                const auto& tc = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTextureCoord;

                list.add ({ { scale * v.x, scale * v.y, scale * v.z, },
                            { scale * n.x, scale * n.y, scale * n.z, },
                            { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                            { tc.x, tc.y } });                               // [8]
            }
        }
    };

    WavefrontObjFile shapeFile;
    OwnedArray<VertexBuffer> vertexBuffers;
};
