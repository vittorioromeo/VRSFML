#if 1 // TODO P0:

int main()
{
}

#else

    ////////////////////////////////////////////////////////////
    // Headers
    ////////////////////////////////////////////////////////////
    #include "SFML/Window/Window.hpp"

    #include "SFML/System/Angle.hpp"
    #include "SFML/System/Clock.hpp"
    #include "SFML/System/Time.hpp"

    #include <X11/Xlib.h>
    #define GLAD_GL_IMPLEMENTATION
    #include "SFML/System/IO.hpp"

    #include "SFML/Base/Array.hpp"

    #include <gl.h>

    #include <array>

    #include <cmath>
    #include <cstdlib>


////////////////////////////////////////////////////////////
/// Initialize OpenGL states into the specified view
///
/// \param Window Target window to initialize
///
/// \return `true` if operation was successful, `false` otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool initialize(sf::Window& window)
{
    // Activate the window
    if (!window.setActive())
    {
        sf::cErr() << "Failed to set the window as active" << sf::endL;
        return false;
    }

    // Setup OpenGL states
    // Set color and depth clear value

    #ifdef SFML_OPENGL_ES
    glClearDepthf(1.f);
    #else
    glClearDepth(1.f);
    #endif

    glClearColor(0.f, 0.5f, 0.5f, 0.f);

    // Enable Z-buffer read and write
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    // Setup a perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const float extent = std::tan(sf::degrees(45).asRadians());

    glFrustum(-extent, extent, -extent, extent, 1.f, 500.f);

    // Enable position and texture coordinates vertex components
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    return true;
}


////////////////////////////////////////////////////////////
/// Draw the OpenGL scene (a rotating cube) into
/// the specified view
///
/// \param window      Target window for rendering
/// \param elapsedTime Time elapsed since the last draw
///
/// \return `true` if operation was successful, `false` otherwise
///
////////////////////////////////////////////////////////////
[[nodiscard]] bool draw(sf::Window& window, float elapsedTime)
{
    // Activate the window
    if (!window.setActive())
    {
        sf::cErr() << "Failed to set the window as active" << sf::endL;
        return false;
    }

    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Apply some transformations
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.f, 0.f, -200.f);
    glRotatef(elapsedTime * 10.f, 1.f, 0.f, 0.f);
    glRotatef(elapsedTime * 6.f, 0.f, 1.f, 0.f);
    glRotatef(elapsedTime * 18.f, 0.f, 0.f, 1.f);

    // Define a 3D cube (6 faces made of 2 triangles composed by 3 vertices)
    // clang-format off
    constexpr sf::base::Array<GLfloat, 216> cube =
    {
        // positions    // colors
        -50, -50, -50,  1, 1, 0,
        -50,  50, -50,  1, 1, 0,
        -50, -50,  50,  1, 1, 0,
        -50, -50,  50,  1, 1, 0,
        -50,  50, -50,  1, 1, 0,
        -50,  50,  50,  1, 1, 0,

         50, -50, -50,  1, 1, 0,
         50,  50, -50,  1, 1, 0,
         50, -50,  50,  1, 1, 0,
         50, -50,  50,  1, 1, 0,
         50,  50, -50,  1, 1, 0,
         50,  50,  50,  1, 1, 0,

        -50, -50, -50,  1, 0, 1,
         50, -50, -50,  1, 0, 1,
        -50, -50,  50,  1, 0, 1,
        -50, -50,  50,  1, 0, 1,
         50, -50, -50,  1, 0, 1,
         50, -50,  50,  1, 0, 1,

        -50,  50, -50,  1, 0, 1,
         50,  50, -50,  1, 0, 1,
        -50,  50,  50,  1, 0, 1,
        -50,  50,  50,  1, 0, 1,
         50,  50, -50,  1, 0, 1,
         50,  50,  50,  1, 0, 1,

        -50, -50, -50,  0, 1, 1,
         50, -50, -50,  0, 1, 1,
        -50,  50, -50,  0, 1, 1,
        -50,  50, -50,  0, 1, 1,
         50, -50, -50,  0, 1, 1,
         50,  50, -50,  0, 1, 1,

        -50, -50,  50,  0, 1, 1,
         50, -50,  50,  0, 1, 1,
        -50,  50,  50,  0, 1, 1,
        -50,  50,  50,  0, 1, 1,
         50, -50,  50,  0, 1, 1,
         50,  50,  50,  0, 1, 1
    };
    // clang-format on

    // Draw the cube
    glVertexPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), cube.data());
    glColorPointer(3, GL_FLOAT, 6 * sizeof(GLfloat), cube.data() + 3);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    return true;
}


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Error code
///
////////////////////////////////////////////////////////////
int main()
{
    // Open a connection with the X server
    Display* display = XOpenDisplay(nullptr);
    if (!display)
        return EXIT_FAILURE;

    // Get the default screen
    const int screen = DefaultScreen(display);

    // Create the main window
    XSetWindowAttributes attributes;
    attributes.background_pixel = BlackPixel(display, screen);
    attributes.event_mask       = KeyPressMask;
    const Window window         = XCreateWindow(display,
                                        RootWindow(display, screen),
                                        0,
                                        0,
                                        650,
                                        330,
                                        0,
                                        DefaultDepth(display, screen),
                                        InputOutput,
                                        DefaultVisual(display, screen),
                                        CWBackPixel | CWEventMask,
                                        &attributes);
    if (!window)
        return EXIT_FAILURE;

    // Set the window's name
    XStoreName(display, window, "SFML Window");

    // Create the windows which will serve as containers for our SFML views
    const Window view1 = XCreateWindow(display,
                                       window,
                                       10,
                                       10,
                                       310,
                                       310,
                                       0,
                                       DefaultDepth(display, screen),
                                       InputOutput,
                                       DefaultVisual(display, screen),
                                       0,
                                       nullptr);
    const Window view2 = XCreateWindow(display,
                                       window,
                                       330,
                                       10,
                                       310,
                                       310,
                                       0,
                                       DefaultDepth(display, screen),
                                       InputOutput,
                                       DefaultVisual(display, screen),
                                       0,
                                       nullptr);

    // Show our windows
    XMapWindow(display, window);
    XFlush(display);

    {
        // Create our SFML views
        sf::Window sfmlView1(view1);
        sf::Window sfmlView2(view2);

        // Create a clock for measuring elapsed time
        const sf::Clock clock;

        // Load OpenGL or OpenGL ES entry points using glad
        if (!sfmlView1.setActive())
        {
            sf::cErr() << "Failed to set view 1 as active" << sf::endL;
            return EXIT_FAILURE;
        }

        // Load OpenGL or OpenGL ES entry points using glad
        sf::WindowContext::loadGLEntryPointsViaGLAD();

        // Initialize our views
        if (!initialize(sfmlView1))
        {
            sf::cErr() << "Failed to initialize view 1" << sf::endL;
            return EXIT_FAILURE;
        }

        if (!initialize(sfmlView2))
        {
            sf::cErr() << "Failed to initialize view 2" << sf::endL;
            return EXIT_FAILURE;
        }

        // Start the event loop
        bool running = true;
        while (running)
        {
            while (XPending(display))
            {
                // Get the next pending event
                XEvent event;
                XNextEvent(display, &event);

                // Process it
                switch (event.type)
                {
                    // Any key is pressed: quit
                    case KeyPress:
                        running = false;
                        break;
                }
            }

            // Draw something into our views
            if (!draw(sfmlView1, clock.getElapsedTime().asSeconds()))
            {
                sf::cErr() << "Failed to draw on view 1" << sf::endL;
                return EXIT_FAILURE;
            }

            if (!draw(sfmlView2, clock.getElapsedTime().asSeconds() * 0.3f))
            {
                sf::cErr() << "Failed to draw on view 2" << sf::endL;
                return EXIT_FAILURE;
            }

            // Display the views on screen
            sfmlView1.display();
            sfmlView2.display();
        }

        // Close our SFML views before destroying the underlying window
    }

    // Destroy the window
    XDestroyWindow(display, window);

    // Close the display
    XCloseDisplay(display);
}

#endif
