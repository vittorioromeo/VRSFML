#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief RAII guard that saves and restores the current shader program binding
///
/// On construction, queries `GL_CURRENT_PROGRAM` and stores its value.
/// On destruction, restores it via `glUseProgram`. Useful when a
/// function needs to bind its own shader temporarily without
/// disturbing surrounding rendering state.
///
////////////////////////////////////////////////////////////
class ShaderSaver
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor: snapshot the current shader program
    ///
    ////////////////////////////////////////////////////////////
    ShaderSaver();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor: restore the previously saved shader program
    ///
    ////////////////////////////////////////////////////////////
    ~ShaderSaver();

    ////////////////////////////////////////////////////////////
    ShaderSaver(const ShaderSaver&)            = delete;
    ShaderSaver& operator=(const ShaderSaver&) = delete;

    ////////////////////////////////////////////////////////////
    ShaderSaver(ShaderSaver&&)            = delete;
    ShaderSaver& operator=(ShaderSaver&&) = delete;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    int m_shaderBinding; //!< Saved `GL_CURRENT_PROGRAM` to restore
};

} // namespace sf::priv
