
//==============================================================================
#pragma once

#include "util/WavefrontObjParser.h"

struct Vertex {
    float position[3];
    float normal[3];
    float colour[4];
    float texCoord[2];
};

//==============================================================================
// This class just manages the attributes that the shaders use.
struct Attributes {
    Attributes(OpenGLContext &context, OpenGLShaderProgram &shaderProgram) {
        position.reset(createAttribute(context, shaderProgram, "position"));
        normal.reset(createAttribute(context, shaderProgram, "normal"));
        sourceColour.reset(createAttribute(context, shaderProgram, "sourceColour"));
        textureCoordIn.reset(createAttribute(context, shaderProgram, "textureCoordIn"));
    }

    void enable(OpenGLContext &context) {
        if (position.get() != nullptr) {
            context.extensions.glVertexAttribPointer(position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
            context.extensions.glEnableVertexAttribArray(position->attributeID);
        }

        if (normal.get() != nullptr) {
            context.extensions.glVertexAttribPointer(normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                                     (GLvoid *) (sizeof(float) * 3));
            context.extensions.glEnableVertexAttribArray(normal->attributeID);
        }

        if (sourceColour.get() != nullptr) {
            context.extensions.glVertexAttribPointer(sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                                     (GLvoid *) (sizeof(float) * 6));
            context.extensions.glEnableVertexAttribArray(sourceColour->attributeID);
        }

        if (textureCoordIn.get() != nullptr) {
            context.extensions.glVertexAttribPointer(textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                                     (GLvoid *) (sizeof(float) * 10));
            context.extensions.glEnableVertexAttribArray(textureCoordIn->attributeID);
        }
    }

    void disable(OpenGLContext &context) {
        if (position.get() != nullptr) context.extensions.glDisableVertexAttribArray(position->attributeID);
        if (normal.get() != nullptr) context.extensions.glDisableVertexAttribArray(normal->attributeID);
        if (sourceColour.get() != nullptr) context.extensions.glDisableVertexAttribArray(sourceColour->attributeID);
        if (textureCoordIn.get() != nullptr) context.extensions.glDisableVertexAttribArray(textureCoordIn->attributeID);
    }

    std::unique_ptr<OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn;

private:
    static OpenGLShaderProgram::Attribute *createAttribute(OpenGLContext &context,
                                                           OpenGLShaderProgram &shader,
                                                           const String &attributeName) {
        if (context.extensions.glGetAttribLocation(shader.getProgramID(), attributeName.toRawUTF8()) < 0)
            return nullptr;

        return new OpenGLShaderProgram::Attribute(shader, attributeName.toRawUTF8());
    }
};

//==============================================================================
// This class just manages the uniform values that the demo shaders use.
struct Uniforms {
    Uniforms(OpenGLContext &context, OpenGLShaderProgram &shaderProgram) {
        projectionMatrix.reset(createUniform(context, shaderProgram, "projectionMatrix"));
        viewMatrix.reset(createUniform(context, shaderProgram, "viewMatrix"));
        time.reset(createUniform(context, shaderProgram, "time"));
    }

    std::unique_ptr<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, time;

private:
    static OpenGLShaderProgram::Uniform *createUniform(OpenGLContext &context,
                                                       OpenGLShaderProgram &shaderProgram,
                                                       const String &uniformName) {
        if (context.extensions.glGetUniformLocation(shaderProgram.getProgramID(), uniformName.toRawUTF8()) < 0)
            return nullptr;

        return new OpenGLShaderProgram::Uniform(shaderProgram, uniformName.toRawUTF8());
    }
};

//==============================================================================
/** This loads a 3D model from an OBJ file and converts it into some vertex buffers
    that we can draw.
*/
struct Shape {
    Shape(OpenGLContext &context, const String& assetName) {
        auto dir = File::getCurrentWorkingDirectory();

        int numTries = 0;

        while (!dir.getChildFile("Resources").exists() && numTries++ < 15)
            dir = dir.getParentDirectory();

        if (shapeFile.load(dir.getChildFile("Resources").getChildFile(assetName)).wasOk())
            for (auto *s : shapeFile.shapes)
                vertexBuffers.add(new VertexBuffer(context, *s));
    }

    void draw(OpenGLContext &context, Attributes &glAttributes) {
        for (auto *vertexBuffer : vertexBuffers) {
            vertexBuffer->bind();

            glAttributes.enable(context);
            glDrawElements(GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, 0);
            glAttributes.disable(context);
        }
    }

private:
    struct VertexBuffer {
        VertexBuffer(OpenGLContext &context, WavefrontObjFile::Shape &aShape) : openGLContext(context) {
            numIndices = aShape.mesh.indices.size();

            openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
            openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

            Array<Vertex> vertices;
            createVertexListFromMesh(aShape.mesh, vertices, Colours::green);

            openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER,
                                                  static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) *
                                                                           sizeof(Vertex)),
                                                  vertices.getRawDataPointer(), GL_STATIC_DRAW);

            openGLContext.extensions.glGenBuffers(1, &indexBuffer);
            openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            openGLContext.extensions.glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                                  static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) *
                                                                           sizeof(juce::uint32)),
                                                  aShape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
        }

        ~VertexBuffer() {
            openGLContext.extensions.glDeleteBuffers(1, &vertexBuffer);
            openGLContext.extensions.glDeleteBuffers(1, &indexBuffer);
        }

        void bind() {
            openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            openGLContext.extensions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        }

        GLuint vertexBuffer, indexBuffer;
        int numIndices;
        OpenGLContext &openGLContext;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
    };

    WavefrontObjFile shapeFile;
    OwnedArray<VertexBuffer> vertexBuffers;

    static void createVertexListFromMesh(const WavefrontObjFile::Mesh &mesh, Array<Vertex> &list, Colour colour) {
        auto scale = 0.2f;
        WavefrontObjFile::TextureCoord defaultTexCoord{0.5f, 0.5f};
        WavefrontObjFile::Vertex defaultNormal{0.5f, 0.5f, 0.5f};

        for (auto i = 0; i < mesh.vertices.size(); ++i) {
            const auto &v = mesh.vertices.getReference(i);
            const auto &n = i < mesh.normals.size() ? mesh.normals.getReference(i) : defaultNormal;
            const auto &tc = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference(i) : defaultTexCoord;

            list.add({{scale * v.x,          scale * v.y,            scale * v.z,},
                      {scale * n.x,          scale * n.y,            scale * n.z,},
                      {colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha()},
                      {tc.x,                 tc.y}});
        }
    }
};