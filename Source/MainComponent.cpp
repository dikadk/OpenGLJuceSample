/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() {
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(800, 600);
    //setFramesPerSecond(60); // This sets the frequency of the update calls.


    addAndMakeVisible(glComponent);
}

MainComponent::~MainComponent() {
}

//==============================================================================
/*void MainComponent::update() {

}*/

//==============================================================================
void MainComponent::paint(Graphics &g) {
    /*g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    float baseAngle = getFrameCounter() * 0.04f;
    for (int i = 0; i < 12; ++i) {
        float angleOffset = 2.f * MathConstants<float>::pi * i / 12.f;
        juce::Point<float> p(getWidth() / 2.0f + radius * std::sin(baseAngle + angleOffset),
                       getHeight() / 2.0f + radius * std::cos(baseAngle + angleOffset));
        g.fillEllipse(p.x, p.y, radius/2, radius/2);
    }*/
}

void MainComponent::resized() {
    ballX = getWidth() / 2;
    ballY = getHeight() / 2;

    glComponent.setBounds(getLocalBounds());
}
