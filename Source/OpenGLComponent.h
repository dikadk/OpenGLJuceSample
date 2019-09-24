/*
  ==============================================================================

    OpenGLComponent.h
    Created: 23 Sep 2019 11:24:29am
    Author:  Dmytro Kiro

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Containters.h"

class OpenGLComponent : public OpenGLAppComponent {
public:
    OpenGLComponent();

    ~OpenGLComponent();

    void paint(Graphics &) override;

    void resized() override;

    void initialise() override;

    void shutdown() override;

    void render() override;

private:
    String vertexShader;
    String fragmentShader;

    std::unique_ptr<OpenGLShaderProgram> shader;
    std::unique_ptr<Shape> shape;
    std::unique_ptr<Attributes> attributes;
    std::unique_ptr<Uniforms> uniforms;

    Matrix3D<float> getProjectionMatrix() const
    {
        auto w = 1.0f / (0.5f + 0.1f);
        auto h = w * getLocalBounds().toFloat().getAspectRatio (false);

        return Matrix3D<float>::fromFrustum(-w, w, -h, h, 4.0f, 30.f); //This gives us a perspective projection as opposed to an orthographic projection.
    };

    Matrix3D<float> getViewMatrix() const
    {
        Matrix3D<float> viewMatrix ({ 0.0f, 0.0f, -10.0f });
        Matrix3D<float> rotationMatrix = viewMatrix.rotation ({ 0.1f, 5.0f * std::sin (getFrameCounter() * 0.01f), 0.f });

        return rotationMatrix * viewMatrix;
    }

    void createShaders(const char *vertexShaderPath, const char *fragmentShaderPath);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponent)
};
