////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/GraphicsContext.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderStates.hpp"
#include "SFML/Graphics/RenderWindow.hpp"

#include "SFML/Window/EventUtils.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Vec2.hpp"

#include "ExampleUtils.hpp"

#include <cstdlib>


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create the graphics context
    auto graphicsContext = sf::GraphicsContext::create().value();

    // Create the window of the application with a stencil buffer
    constexpr sf::Vec2f windowSize{600.f, 600.f};

    auto window = makeDPIScaledRenderWindow({
        .size            = windowSize.toVec2u(),
        .title           = "SFML Stencil",
        .resizable       = true,
        .vsync           = true,
        .contextSettings = {.depthBits = 0u, .stencilBits = 8u},
    });

    const sf::RectangleShape red({
        .position{270.f, 70.f},
        .rotation  = sf::degrees(60.f),
        .fillColor = sf::Color::Red,
        .size      = {500.f, 50.f},
    });

    const sf::RectangleShape green({
        .position{370.f, 100.f},
        .rotation  = sf::degrees(120.f),
        .fillColor = sf::Color::Green,
        .size      = {500.f, 50.f},
    });

    const sf::RectangleShape blue({
        .position{550.f, 470.f},
        .rotation  = sf::degrees(180.f),
        .fillColor = sf::Color::Blue,
        .size      = {500.f, 50.f},
    });

    while (true)
    {
        // Handle events
        while (const sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, windowSize, window))
                continue;
        }

        // When drawing using a 2D API, we normally resort to what is known as the "painter's algorithm".
        // Because our graphics primitives lack any depth information, objects that are drawn later in the frame will
        // overlap objects drawn earlier in the frame. This means that the objects have to be sorted from farthest to
        // closest and drawn in that order to appear correct.
        // This also means that objects cannot simultaneously be both "in front" and "behind" already drawn objects.

        // With the magic of the stencil buffer we can get around this rule. Much like the depth buffer in 3D applications
        // the stencil buffer holds additional information that informs the rendering pipeline about our intentions.
        // Unlike the depth buffer which requires that all drawn primitives contain depth information (e.g. in the form
        // of 3D vertices), the stencil buffer allows us to specify stencil values for whole primitives ourselves.

        // For every fragment/pixel that would be written to the screen, after the fragment shader is executed, the
        // stencil test is performed. First, the rendering pipeline will ask whether the pixel in question should
        // even be kept or discarded. This is known as the stencil test. In order to answer this question 2 integer
        // values are compared against each other, the value already in the screen stencil buffer corresponding to
        // the pixel in question and the new value of the incoming pixel to perform the test for. The value of the
        // incoming pixel is known as the reference value and can be set per draw operation when using the
        // sfml-graphics drawing API. All mathematical operations comparing 2 integers are supported: Less,
        // LessEqual, Greater, GreaterEqual, Equal, NotEqual. Additionally, 2 special comparisons are provided:
        // Always and Never. Always will make sure the stencil test will always pass, whereas Never will make sure
        // the stencil test never passes. The incoming reference value is compared to the stencil buffer value in
        // the following order: (ReferenceValue Comparison BufferValue) If the test evaluates to true, the pixel is
        // kept, otherwise it is culled and will no longer contribute to the frame in any way.

        // Once the stencil test passes, the value in the stencil buffer can be updated with a new value. The new
        // value is determined by the update operation and for the Replace operation the incoming reference value as
        // well. In the case of Increment, Decrement and Invert, the existing value in the stencil buffer is
        // modified accordingly, Invert will perform a bit-wise inversion of the integer value in the buffer. Keep
        // will not modify the value in the buffer whereas Zero will set it to 0. Replace will replace the value in
        // the buffer with the incoming reference value.

        // Like all data types, the stencil values in the stencil buffer have a finite bit width. Typically stencil
        // buffers with 8-bits are offered by the graphics implementation. In complex scenarios, we might want to
        // partition our bits up into multiple areas so a single stencil buffer value can be used for multiple
        // purposes simultaneously. For this purpose, we can specify a mask value that is bit-wise ANDed with both
        // the incoming reference value and the stencil buffer value before they are compared. For simple cases, a
        // mask of ~0 (all 1s) can be used which is the equivalent of disabling masking all together.

        // For certain effects, objects might have to be rendered multiple times. Once to establish the stencil
        // value of that object within the stencil buffer and another to draw the object itself including its
        // texture/color. Drawing objects with the sole purpose of updating stencil buffer values is also known as
        // performing a stencil-only pass. Skipping texturing and writes to the color buffer can save a lot of time
        // depending on the object to be drawn. StencilMode allows us to perform stencil-only drawing by setting the
        // corresponding flag to true.

        // In this example, we demonstrate how can can draw 3 cyclically overlapping rectangles using the stencil
        // buffer. Without the stencil buffer, 1 of the rectangles would have to be precisely split along the edge
        // of another rectangle and both pieces would have to be drawn at different stages in the draw pass. This
        // would not only be almost impossible to compute to the required accuracy to mimic the GPU's vertex
        // computations but splitting a primitive up and drawing the pieces in separate draw calls would introduce
        // noticeable artifacts which would reduce the overall quality of the output image.

        // To start with, we initialize the stencil buffer values for every pixel to 0 at the start of each frame.
        // In our draw calls we need to make sure that the stencil reference values of all objects will pass the
        // test compared to the initial buffer value. In the case of Always, the initial value is insignificant,
        // when we use Greater we make sure the reference value of 2 is greater than 0.

        // Clear the window color to black and the initial stencil buffer values to 0
        window.clear(sf::Color::Black, sf::StencilValue{0u});

        // Draw rectangles

        // We draw the first rectangle with comparison set to always so that it will definitely draw and update
        // (Replace) the stencil buffer values of its pixels to the specified reference value.
        window.draw(red,
                    sf::RenderStates{.stencilMode = {
                                         .stencilComparison      = sf::StencilComparison::Always,
                                         .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                                         .stencilOnly            = false,
                                         .stencilReference       = sf::StencilValue{3u},
                                         .stencilMask            = sf::StencilValue{~0u},
                                     }});

        // Just like the first, we draw the second rectangle with comparison set to always so that it will definitely
        // draw and update (Replace) the stencil buffer values of its pixels to the specified reference value.
        // In the case of pixels overlapping the first rectangle, because we specify Always as the comparison, it is
        // as if we are drawing using the painter's algorithm, i.e. newer pixels overwrite older pixels.
        window.draw(green,
                    sf::RenderStates{.stencilMode = {
                                         .stencilComparison      = sf::StencilComparison::Always,
                                         .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                                         .stencilOnly            = false,
                                         .stencilReference       = sf::StencilValue{1u},
                                         .stencilMask            = sf::StencilValue{~0u},
                                     }});

        // Now comes the magic. We want to draw the third rectangle so it is behind i.e. does not overwrite pixels
        // of the first rectangle but in front of i.e. overwrites pixels of the second rectangle. We already set the
        // reference value of the first rectangle to 3 and the second rectangle to 1, so in order to be "between"
        // them, this rectangle has to have a reference value of 2. 2 is not greater than 3 so pixels of this
        // rectangle will not overwrite pixels of the first rectangle, however 2 is greater than 1 and thus pixels
        // of this rectangle will overwrite pixels of the second rectangle. The stencil update operation for this
        // draw operation is not significant in any way since this is the last draw call in the frame.
        window.draw(blue,
                    sf::RenderStates{.stencilMode = {
                                         .stencilComparison      = sf::StencilComparison::Greater,
                                         .stencilUpdateOperation = sf::StencilUpdateOperation::Replace,
                                         .stencilOnly            = false,
                                         .stencilReference       = sf::StencilValue{2u},
                                         .stencilMask            = sf::StencilValue{~0u},
                                     }});

        // Display things on screen
        window.display();
    }

    return 0;
}
