/*
  ==============================================================================

    OpenGLComponent.cpp
    Created: 23 Sep 2019 11:24:29am
    Author:  Dmytro Kiro

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "OpenGLComponent.h"

//==============================================================================
OpenGLComponent::OpenGLComponent() {
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

OpenGLComponent::~OpenGLComponent() {
    shutdownOpenGL();
}

void OpenGLComponent::paint(Graphics &g) {

}

void OpenGLComponent::resized() {
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void OpenGLComponent::initialise() {
    createShaders("/Users/dkiro/Desktop/test11/SampleAnimation/Source/shaders/shader.vs",
                  "/Users/dkiro/Desktop/test11/SampleAnimation/Source/shaders/shader.fs");
}

void OpenGLComponent::shutdown() {
    shader.reset();
    shape.reset();
    attributes.reset();
    uniforms.reset();
}

void OpenGLComponent::render() {
    jassert (OpenGLHelpers::isContextActive());

    auto desktopScale = (float) openGLContext.getRenderingScale();
}

void OpenGLComponent::createShaders(const char *vertexShaderPath, const char *fragmentShaderPath) {
    File vs(vertexShaderPath);
    vertexShader = vs.loadFileAsString();

    File fs(fragmentShaderPath);
    fragmentShader = fs.loadFileAsString();

    std::unique_ptr<OpenGLShaderProgram> newShader(new OpenGLShaderProgram(openGLContext));
    String statusText;

    if (newShader->addVertexShader(OpenGLHelpers::translateVertexShaderToV3(vertexShader))
        && newShader->addFragmentShader(OpenGLHelpers::translateFragmentShaderToV3(fragmentShader))
        && newShader->link()) {
        shape.reset();
        attributes.reset();
        uniforms.reset();

        shader.reset(newShader.release());
        shader->use();

        shape.reset(new Shape(openGLContext));
        attributes.reset(new Attributes(openGLContext, *shader));
        uniforms.reset(new Uniforms(openGLContext, *shader));

        statusText = "GLSL: v" + String(OpenGLShaderProgram::getLanguageVersion());
    } else
        statusText = newShader->getLastError();
}
