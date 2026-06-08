////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Keyboard.hpp"

#include "ExampleUtils/Scaling.hpp"

#include "Zancle/Audio/AudioContext.hpp"
#include "Zancle/Audio/AudioSettings.hpp" // IWYU pragma: keep
#include "Zancle/Audio/PlaybackDevice.hpp"
#include "Zancle/Audio/Sound.hpp"
#include "Zancle/Audio/SoundBuffer.hpp"

#include "Zancle/Graphics/Color.hpp"
#include "Zancle/Graphics/DrawableBatch.hpp"
#include "Zancle/Graphics/Font.hpp"
#include "Zancle/Graphics/GraphicsContext.hpp"
#include "Zancle/Graphics/PrimitiveType.hpp"
#include "Zancle/Graphics/RenderStates.hpp"
#include "Zancle/Graphics/RenderWindow.hpp"
#include "Zancle/Graphics/Text.hpp"
#include "Zancle/Graphics/TextBase.inl" // IWYU pragma: keep, required: `ShinyText` below derives from `za::Text`
#include "Zancle/Graphics/Transformable.hpp"
#include "Zancle/Graphics/Vertex.hpp"
#include "Zancle/Graphics/View.hpp" // IWYU pragma: keep

#include "Zancle/Window/Event.hpp" // IWYU pragma: keep
#include "Zancle/Window/EventUtils.hpp"
#include "Zancle/Window/WindowSettings.hpp" // IWYU pragma: keep

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"

#include "Zancle/IO/Path.hpp"

#include "Zancle/Chrono/Clock.hpp"
#include "Zancle/Chrono/Time.hpp"

#include "Zancle/Container/Array.hpp"
#include "Zancle/Container/Vector.hpp"

#include "Zancle/Geometry/Priv/Vec2Base.hpp"
#include "Zancle/Geometry/Rect2.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Math/Fabs.hpp"
#include "Zancle/Math/MinMax.hpp"
#include "Zancle/Math/Round.hpp"

#include "Zancle/Base/Abort.hpp"
#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/IntTypes.hpp"
#include "Zancle/Base/Macros.hpp"
#include "Zancle/Base/SizeT.hpp"


#ifdef ZA_SYSTEM_IOS
    #include <Zancle/Main.hpp>
#endif


namespace
{
za::Path resourcesDir()
{
#ifdef ZA_SYSTEM_IOS
    return "";
#else
    return "resources";
#endif
}

// Get the C++ enumerator name of the given `za::Keyboard::Key` value including `Key::` prefix
constexpr const char* keyIdentifier(za::Keyboard::Key code)
{
    switch (code)
    {
#define CASE(code)                \
    case za::Keyboard::Key::code: \
        return "Key::" #code
        CASE(Unknown);
        CASE(A);
        CASE(B);
        CASE(C);
        CASE(D);
        CASE(E);
        CASE(F);
        CASE(G);
        CASE(H);
        CASE(I);
        CASE(J);
        CASE(K);
        CASE(L);
        CASE(M);
        CASE(N);
        CASE(O);
        CASE(P);
        CASE(Q);
        CASE(R);
        CASE(S);
        CASE(T);
        CASE(U);
        CASE(V);
        CASE(W);
        CASE(X);
        CASE(Y);
        CASE(Z);
        CASE(Num0);
        CASE(Num1);
        CASE(Num2);
        CASE(Num3);
        CASE(Num4);
        CASE(Num5);
        CASE(Num6);
        CASE(Num7);
        CASE(Num8);
        CASE(Num9);
        CASE(Escape);
        CASE(LControl);
        CASE(LShift);
        CASE(LAlt);
        CASE(LSystem);
        CASE(RControl);
        CASE(RShift);
        CASE(RAlt);
        CASE(RSystem);
        CASE(Menu);
        CASE(LBracket);
        CASE(RBracket);
        CASE(Semicolon);
        CASE(Comma);
        CASE(Period);
        CASE(Apostrophe);
        CASE(Slash);
        CASE(Backslash);
        CASE(Grave);
        CASE(Equal);
        CASE(Hyphen);
        CASE(Space);
        CASE(Enter);
        CASE(Backspace);
        CASE(Tab);
        CASE(PageUp);
        CASE(PageDown);
        CASE(End);
        CASE(Home);
        CASE(Insert);
        CASE(Delete);
        CASE(Add);
        CASE(Subtract);
        CASE(Multiply);
        CASE(Divide);
        CASE(Left);
        CASE(Right);
        CASE(Up);
        CASE(Down);
        CASE(Numpad0);
        CASE(Numpad1);
        CASE(Numpad2);
        CASE(Numpad3);
        CASE(Numpad4);
        CASE(Numpad5);
        CASE(Numpad6);
        CASE(Numpad7);
        CASE(Numpad8);
        CASE(Numpad9);
        CASE(F1);
        CASE(F2);
        CASE(F3);
        CASE(F4);
        CASE(F5);
        CASE(F6);
        CASE(F7);
        CASE(F8);
        CASE(F9);
        CASE(F10);
        CASE(F11);
        CASE(F12);
        CASE(F13);
        CASE(F14);
        CASE(F15);
        CASE(Pause);
        CASE(Exclaim);
        CASE(DoubleApostrophe);
        CASE(Hash);
        CASE(Dollar);
        CASE(Percent);
        CASE(Ampersand);
        CASE(LParenthesis);
        CASE(RParenthesis);
        CASE(Asterisk);
        CASE(Plus);
        CASE(Colon);
        CASE(Less);
        CASE(Greater);
        CASE(Question);
        CASE(At);
        CASE(Caret);
        CASE(Underscore);
        CASE(LBrace);
        CASE(Pipe);
        CASE(RBrace);
        CASE(Tilde);
        CASE(PlusMinus);
        CASE(CapsLock);
        CASE(PrintScreen);
        CASE(ScrollLock);
        CASE(NumLock);
        CASE(NumpadEnter);
        CASE(NumpadPeriod);
        CASE(Application);
        CASE(Power);
        CASE(NumpadEqual);
        CASE(F16);
        CASE(F17);
        CASE(F18);
        CASE(F19);
        CASE(F20);
        CASE(F21);
        CASE(F22);
        CASE(F23);
        CASE(F24);
        CASE(Execute);
        CASE(Help);
        CASE(Select);
        CASE(Stop);
        CASE(Redo);
        CASE(Undo);
        CASE(Cut);
        CASE(Copy);
        CASE(Paste);
        CASE(Find);
        CASE(VolumeMute);
        CASE(VolumeUp);
        CASE(VolumeDown);
        CASE(NumpadComma);
        CASE(NumpadEqualAs400);
        CASE(AltErase);
        CASE(SysReq);
        CASE(Cancel);
        CASE(Clear);
        CASE(Prior);
        CASE(Enter2);
        CASE(Separator);
        CASE(Out);
        CASE(Oper);
        CASE(ClearAgain);
        CASE(CrSel);
        CASE(ExSel);
        CASE(Numpad00);
        CASE(Numpad000);
        CASE(ThousandsSeparator);
        CASE(DecimalSeparator);
        CASE(CurrencyUnit);
        CASE(CurrencySubUnit);
        CASE(NumpadLParenthesis);
        CASE(NumpadRParenthesis);
        CASE(NumpadLBrace);
        CASE(NumpadRBrace);
        CASE(NumpadTab);
        CASE(NumpadBackspace);
        CASE(NumpadA);
        CASE(NumpadB);
        CASE(NumpadC);
        CASE(NumpadD);
        CASE(NumpadE);
        CASE(NumpadF);
        CASE(NumpadXor);
        CASE(NumpadPower);
        CASE(NumpadPercent);
        CASE(NumpadLess);
        CASE(NumpadGreater);
        CASE(NumpadAmpersand);
        CASE(NumpadDoubleAmpersand);
        CASE(NumpadVerticalBar);
        CASE(NumpadDoubleVerticalBar);
        CASE(NumpadColon);
        CASE(NumpadHash);
        CASE(NumpadSpace);
        CASE(NumpadAt);
        CASE(NumpadExclamation);
        CASE(NumpadMemStore);
        CASE(NumpadMemRecall);
        CASE(NumpadMemClear);
        CASE(NumpadMemAdd);
        CASE(NumpadMemSubtract);
        CASE(NumpadMemMultiply);
        CASE(NumpadMemDivide);
        CASE(NumpadPlusMinus);
        CASE(NumpadClear);
        CASE(NumpadClearEntry);
        CASE(NumpadBinary);
        CASE(NumpadOctal);
        CASE(NumpadDecimal);
        CASE(NumpadHexadecimal);
        CASE(ModeChange);
        CASE(Sleep);
        CASE(Wake);
        CASE(ChannelIncrement);
        CASE(ChannelDecrement);
        CASE(MediaPlay);
        CASE(MediaPause);
        CASE(MediaRecord);
        CASE(MediaFastForward);
        CASE(MediaRewind);
        CASE(MediaNextTrack);
        CASE(MediaPreviousTrack);
        CASE(MediaStop);
        CASE(MediaEject);
        CASE(MediaPlayPause);
        CASE(LaunchMediaSelect);
        CASE(AcNew);
        CASE(AcOpen);
        CASE(AcClose);
        CASE(AcExit);
        CASE(AcSave);
        CASE(AcPrint);
        CASE(AcProperties);
        CASE(Search);
        CASE(HomePage);
        CASE(Back);
        CASE(Forward);
        CASE(AcStop);
        CASE(Refresh);
        CASE(Favorites);
        CASE(SoftLeft);
        CASE(SoftRight);
        CASE(Call);
        CASE(EndCall);
        CASE(LTab);
        CASE(Level5Shift);
        CASE(MultiKeyCompose);
        CASE(LMeta);
        CASE(RMeta);
        CASE(LHyper);
        CASE(RHyper);
#undef CASE
    }

    za::printErr("invalid keyboard code");
    za::abort();
}

// Get the C++ enumerator name of the given `za::Keyboard::Scancode` value including `Scan::` prefix
constexpr const char* scancodeIdentifier(za::Keyboard::Scancode scancode)
{
    switch (scancode)
    {
#define CASE(scancode)                 \
    case za::Keyboard::Scan::scancode: \
        return "Scan::" #scancode
        CASE(Unknown);
        CASE(A);
        CASE(B);
        CASE(C);
        CASE(D);
        CASE(E);
        CASE(F);
        CASE(G);
        CASE(H);
        CASE(I);
        CASE(J);
        CASE(K);
        CASE(L);
        CASE(M);
        CASE(N);
        CASE(O);
        CASE(P);
        CASE(Q);
        CASE(R);
        CASE(S);
        CASE(T);
        CASE(U);
        CASE(V);
        CASE(W);
        CASE(X);
        CASE(Y);
        CASE(Z);
        CASE(Num1);
        CASE(Num2);
        CASE(Num3);
        CASE(Num4);
        CASE(Num5);
        CASE(Num6);
        CASE(Num7);
        CASE(Num8);
        CASE(Num9);
        CASE(Num0);
        CASE(Enter);
        CASE(Escape);
        CASE(Backspace);
        CASE(Tab);
        CASE(Space);
        CASE(Hyphen);
        CASE(Equal);
        CASE(LBracket);
        CASE(RBracket);
        CASE(Backslash);
        CASE(Semicolon);
        CASE(Apostrophe);
        CASE(Grave);
        CASE(Comma);
        CASE(Period);
        CASE(Slash);
        CASE(F1);
        CASE(F2);
        CASE(F3);
        CASE(F4);
        CASE(F5);
        CASE(F6);
        CASE(F7);
        CASE(F8);
        CASE(F9);
        CASE(F10);
        CASE(F11);
        CASE(F12);
        CASE(F13);
        CASE(F14);
        CASE(F15);
        CASE(F16);
        CASE(F17);
        CASE(F18);
        CASE(F19);
        CASE(F20);
        CASE(F21);
        CASE(F22);
        CASE(F23);
        CASE(F24);
        CASE(CapsLock);
        CASE(PrintScreen);
        CASE(ScrollLock);
        CASE(Pause);
        CASE(Insert);
        CASE(Home);
        CASE(PageUp);
        CASE(Delete);
        CASE(End);
        CASE(PageDown);
        CASE(Right);
        CASE(Left);
        CASE(Down);
        CASE(Up);
        CASE(NumLock);
        CASE(NumpadDivide);
        CASE(NumpadMultiply);
        CASE(NumpadMinus);
        CASE(NumpadPlus);
        CASE(NumpadEqual);
        CASE(NumpadEnter);
        CASE(NumpadDecimal);
        CASE(Numpad1);
        CASE(Numpad2);
        CASE(Numpad3);
        CASE(Numpad4);
        CASE(Numpad5);
        CASE(Numpad6);
        CASE(Numpad7);
        CASE(Numpad8);
        CASE(Numpad9);
        CASE(Numpad0);
        CASE(NonUsBackslash);
        CASE(Application);
        CASE(Execute);
        CASE(ModeChange);
        CASE(Help);
        CASE(Menu);
        CASE(Select);
        CASE(Redo);
        CASE(Undo);
        CASE(Cut);
        CASE(Copy);
        CASE(Paste);
        CASE(VolumeMute);
        CASE(VolumeUp);
        CASE(VolumeDown);
        CASE(MediaPlayPause);
        CASE(MediaStop);
        CASE(MediaNextTrack);
        CASE(MediaPreviousTrack);
        CASE(LControl);
        CASE(LShift);
        CASE(LAlt);
        CASE(LSystem);
        CASE(RControl);
        CASE(RShift);
        CASE(RAlt);
        CASE(RSystem);
        CASE(Back);
        CASE(Forward);
        CASE(Refresh);
        CASE(Stop);
        CASE(Search);
        CASE(Favorites);
        CASE(HomePage);
        CASE(LaunchMediaSelect);
        CASE(NonUsHash);
        CASE(NumpadPeriod);
        CASE(Power);
        CASE(Find);
        CASE(NumpadComma);
        CASE(NumpadEqualas400);
        CASE(International1);
        CASE(International2);
        CASE(International3);
        CASE(International4);
        CASE(International5);
        CASE(International6);
        CASE(International7);
        CASE(International8);
        CASE(International9);
        CASE(Lang1);
        CASE(Lang2);
        CASE(Lang3);
        CASE(Lang4);
        CASE(Lang5);
        CASE(Lang6);
        CASE(Lang7);
        CASE(Lang8);
        CASE(Lang9);
        CASE(AltErase);
        CASE(SysReq);
        CASE(Cancel);
        CASE(Clear);
        CASE(Prior);
        CASE(Enter2);
        CASE(Separator);
        CASE(Out);
        CASE(Oper);
        CASE(ClearAgain);
        CASE(CrSel);
        CASE(ExSel);
        CASE(Numpad00);
        CASE(Numpad000);
        CASE(ThousandsSeparator);
        CASE(DecimalsSeparator);
        CASE(CurrencyUnit);
        CASE(CurrencySubUnit);
        CASE(NumpadLParenthesis);
        CASE(NumpadRParenthesis);
        CASE(NumpadLBrace);
        CASE(NumpadRBrace);
        CASE(NumpadTab);
        CASE(NumpadBackspace);
        CASE(NumpadA);
        CASE(NumpadB);
        CASE(NumpadC);
        CASE(NumpadD);
        CASE(NumpadE);
        CASE(NumpadF);
        CASE(NumpadXor);
        CASE(NumpadPower);
        CASE(NumpadPercent);
        CASE(NumpadLess);
        CASE(NumpadGreater);
        CASE(NumpadAmpersand);
        CASE(NumpadDoubleAmpersand);
        CASE(NumpadVerticalBar);
        CASE(NumpadDoubleVerticalBar);
        CASE(NumpadColon);
        CASE(NumpadHash);
        CASE(NumpadSpace);
        CASE(NumpadAt);
        CASE(NumpadExclamation);
        CASE(NumpadMemStore);
        CASE(NumpadMemRecall);
        CASE(NumpadMemClear);
        CASE(NumpadMemAdd);
        CASE(NumpadMemSubtract);
        CASE(NumpadMemMultiply);
        CASE(NumpadMemDivide);
        CASE(NumpadPlusMinus);
        CASE(NumpadClear);
        CASE(NumpadClearEntry);
        CASE(NumpadBinary);
        CASE(NumpadOctal);
        CASE(NumpadHexadecimal);
        CASE(Sleep);
        CASE(Wake);
        CASE(ChannelIncrement);
        CASE(ChannelDecrement);
        CASE(MediaPlay);
        CASE(MediaPause);
        CASE(MediaRecord);
        CASE(MediaFastForward);
        CASE(MediaRewind);
        CASE(MediaEject);
        CASE(AcNew);
        CASE(AcOpen);
        CASE(AcClose);
        CASE(AcExit);
        CASE(AcSave);
        CASE(AcPrint);
        CASE(AcProperties);
        CASE(AcStop);
        CASE(SoftLeft);
        CASE(SoftRight);
        CASE(Call);
        CASE(EndCall);
#undef CASE
    }

    za::printErr("invalid keyboard scancode");
    za::abort();
}


////////////////////////////////////////////////////////////
// Entity showing keyboard events and real-time state on a keyboard
////////////////////////////////////////////////////////////
class KeyboardView : public za::Transformable
{
public:
    explicit KeyboardView(const za::Font& font) :
        m_labels(za::Keyboard::ScancodeCount, za::Text(font, {.string = "", .characterSize = 14u}))
    {
        // Check all the scancodes are in the matrix exactly once
        {
            [[maybe_unused]] bool scancodesInMatrix[za::Keyboard::ScancodeCount]{};

            for (const auto& [cells, marginBottom] : m_matrix)
            {
                for (const auto& [scancode, size, marginRight] : cells)
                {
                    ZA_ASSERT(!scancodesInMatrix[static_cast<za::SizeT>(scancode)]);
                    scancodesInMatrix[static_cast<za::SizeT>(scancode)] = true;
                }
            }

            // TODO P1: restore?
            // ZA_ASSERT(scancodesInMatrix.size() == za::Keyboard::ScancodeCount);
        }

        // Initialize keys color and label
        forEachKey([this](za::Keyboard::Scancode scancode, const za::Rect2f& rect)
        {
            const auto scancodeIndex = static_cast<za::SizeT>(scancode);

            for (za::SizeT vertexIndex = 0u; vertexIndex < 6u; ++vertexIndex)
                m_triangles[6u * scancodeIndex + vertexIndex]
                    .color = za::Keyboard::delocalize(za::Keyboard::localize(scancode)) != scancode
                                 ? za::Color::Red
                                 : za::Color::White;

            za::Text& label = m_labels[scancodeIndex];
            label.setString(za::Keyboard::getDescription(scancode));
            label.position = {rect.position + rect.size / 2.f};

            if (rect.size.x < label.getLocalBounds().size.x + padding * 2.f + 2.f)
            {
                za::Utf8String string = label.getString();
                string.replaceAllOccurrences(" ", "\n");
                label.setString(string);
            }
            while (rect.size.x < label.getLocalBounds().size.x + padding * 2.f + 2.f)
                label.setCharacterSize(label.getCharacterSize() - 2);

            const za::Rect2f bounds = label.getLocalBounds();
            label.origin            = {za::round(bounds.position.x + bounds.size.x / 2.f),
                                       za::round(static_cast<float>(label.getCharacterSize()) / 2.f)};
        });
    }

    void handle(const za::Event& event)
    {
        // React to keyboard events by starting an animation
        if (const auto* keyPressed = event.getIf<za::Event::KeyPressed>())
        {
            if (keyPressed->scancode != za::Keyboard::Scan::Unknown)
                m_moveFactors[static_cast<za::SizeT>(keyPressed->scancode)] = 1.f;
        }
        else if (const auto* keyReleased = event.getIf<za::Event::KeyReleased>())
        {
            if (keyReleased->scancode != za::Keyboard::Scan::Unknown)
                m_moveFactors[static_cast<za::SizeT>(keyReleased->scancode)] = -1.f;
        }
    }

    void update(za::Time frameTime)
    {
        // Animate m_moveFactors values linearly towards zero
        static constexpr za::Time transitionDuration = za::milliseconds(200);
        for (float& factor : m_moveFactors)
        {
            const float absoluteChange = za::min(za::fabs(factor), frameTime / transitionDuration);
            factor += factor > 0.f ? -absoluteChange : absoluteChange;
        }

        // Update vertices positions from m_moveFactors and opacity from real-time keyboard state
        forEachKey([this](za::Keyboard::Scancode scancode, const za::Rect2f& rect)
        {
            const auto scancodeIndex = static_cast<za::SizeT>(scancode);

            static constexpr za::Vec2f square[]{
                {0.f, 0.f},
                {1.f, 0.f},
                {1.f, 1.f},
                {0.f, 1.f},
            };

            static constexpr unsigned int cornerIndexes[]{0u, 1u, 3u, 3u, 1u, 2u};

            const float     moveFactor = m_moveFactors[scancodeIndex];
            const za::Vec2f move(0.f, 2.f * moveFactor * (1.f - za::fabs(moveFactor)) * padding);

            const bool pressed = za::Keyboard::isKeyPressed(scancode);

            for (za::SizeT vertexIndex = 0u; vertexIndex < 6u; ++vertexIndex)
            {
                za::Vertex&                vertex = m_triangles[6u * scancodeIndex + vertexIndex];
                const za::Vec2f            corner = square[cornerIndexes[vertexIndex]];
                static constexpr za::Vec2f pad(padding, padding);
                vertex.position = rect.position + pad + (rect.size - 2.f * pad).componentWiseMul(corner) + move;
                vertex.color.a  = pressed ? 96 : 48;
            }

            m_labels[scancodeIndex].position = rect.position + rect.size / 2.f + move;
        });
    }

    void draw(za::RenderTarget& target, za::RenderStates states) const
    {
        states.transform *= getTransform();

        target.draw(m_triangles, za::PrimitiveType::Triangles, states);

        for (const za::Text& label : m_labels)
            target.draw(label, states);
    }

private:
    // Template to iterate on scancodes and the corresponding computed rectangle in local coordinates
    template <typename F>
    void forEachKey(F&& function) const
    {
        za::Vec2f pos;
        for (const auto& [cells, marginBottom] : m_matrix)
        {
            for (const auto& [scancode, size, marginRight] : cells)
            {
                function(scancode, za::Rect2f(pos, size));
                pos.x += size.x + marginRight;
            }
            pos.x = 0.f;
            pos.y += keySize + marginBottom;
        }
    }

    static constexpr float keySize = 54.f;
    static constexpr float padding = 4.f;

    struct Cell
    {
        Cell(za::Keyboard::Scancode theScancode, za::Vec2f sizeRatio = {1.f, 1.f}, float marginRightRatio = 0.f) :
            scancode(theScancode),
            size(sizeRatio * keySize),
            marginRight(marginRightRatio * keySize)
        {
        }

        Cell(za::Keyboard::Scancode theScancode, float marginRightRatio) :
            Cell(theScancode, {1.f, 1.f}, marginRightRatio)
        {
        }

        za::Keyboard::Scancode scancode;
        za::Vec2f              size;
        float                  marginRight;
    };

    struct Row
    {
        Row(za::Vector<Cell> theCells, float marginBottomRatio = 0.f) :
            cells(ZA_MOVE(theCells)),
            marginBottom(marginBottomRatio * keySize)
        {
        }

        za::Vector<Cell> cells;
        float            marginBottom;
    };

    const za::Array<Row, 9> m_matrix{{
        {{{za::Keyboard::Scan::Escape, 1},
          {za::Keyboard::Scan::F1},
          {za::Keyboard::Scan::F2},
          {za::Keyboard::Scan::F3},
          {za::Keyboard::Scan::F4, 0.5},
          {za::Keyboard::Scan::F5},
          {za::Keyboard::Scan::F6},
          {za::Keyboard::Scan::F7},
          {za::Keyboard::Scan::F8, 0.5},
          {za::Keyboard::Scan::F9},
          {za::Keyboard::Scan::F10},
          {za::Keyboard::Scan::F11},
          {za::Keyboard::Scan::F12, 0.5},
          {za::Keyboard::Scan::PrintScreen},
          {za::Keyboard::Scan::ScrollLock},
          {za::Keyboard::Scan::Pause}},
         0.5},
        {{{za::Keyboard::Scan::Grave}, //
          {za::Keyboard::Scan::Num1},
          {za::Keyboard::Scan::Num2},
          {za::Keyboard::Scan::Num3},
          {za::Keyboard::Scan::Num4},
          {za::Keyboard::Scan::Num5},
          {za::Keyboard::Scan::Num6},
          {za::Keyboard::Scan::Num7},
          {za::Keyboard::Scan::Num8},
          {za::Keyboard::Scan::Num9},
          {za::Keyboard::Scan::Num0},
          {za::Keyboard::Scan::Hyphen},
          {za::Keyboard::Scan::Equal},
          {za::Keyboard::Scan::Backspace, {2, 1}, 0.5},
          {za::Keyboard::Scan::Insert},
          {za::Keyboard::Scan::Home},
          {za::Keyboard::Scan::PageUp, 0.5},
          {za::Keyboard::Scan::NumLock},
          {za::Keyboard::Scan::NumpadDivide},
          {za::Keyboard::Scan::NumpadMultiply},
          {za::Keyboard::Scan::NumpadMinus}}},
        {{{za::Keyboard::Scan::Tab, {1.5, 1}},
          {za::Keyboard::Scan::Q},
          {za::Keyboard::Scan::W},
          {za::Keyboard::Scan::E},
          {za::Keyboard::Scan::R},
          {za::Keyboard::Scan::T},
          {za::Keyboard::Scan::Y},
          {za::Keyboard::Scan::U},
          {za::Keyboard::Scan::I},
          {za::Keyboard::Scan::O},
          {za::Keyboard::Scan::P},
          {za::Keyboard::Scan::LBracket},
          {za::Keyboard::Scan::RBracket},
          {za::Keyboard::Scan::Backslash, {1.5, 1}, 0.5},
          {za::Keyboard::Scan::Delete},
          {za::Keyboard::Scan::End},
          {za::Keyboard::Scan::PageDown, 0.5},
          {za::Keyboard::Scan::Numpad7},
          {za::Keyboard::Scan::Numpad8},
          {za::Keyboard::Scan::Numpad9},
          {za::Keyboard::Scan::NumpadPlus}}},
        {{{za::Keyboard::Scan::CapsLock, {1.75, 1}},
          {za::Keyboard::Scan::A},
          {za::Keyboard::Scan::S},
          {za::Keyboard::Scan::D},
          {za::Keyboard::Scan::F},
          {za::Keyboard::Scan::G},
          {za::Keyboard::Scan::H},
          {za::Keyboard::Scan::J},
          {za::Keyboard::Scan::K},
          {za::Keyboard::Scan::L},
          {za::Keyboard::Scan::Semicolon},
          {za::Keyboard::Scan::Apostrophe},
          {za::Keyboard::Scan::Enter, {2.25, 1}, 4},
          {za::Keyboard::Scan::Numpad4},
          {za::Keyboard::Scan::Numpad5},
          {za::Keyboard::Scan::Numpad6},
          {za::Keyboard::Scan::NumpadEqual}}},
        {{{za::Keyboard::Scan::LShift, {1.25, 1}},
          {za::Keyboard::Scan::NonUsBackslash},
          {za::Keyboard::Scan::Z},
          {za::Keyboard::Scan::X},
          {za::Keyboard::Scan::C},
          {za::Keyboard::Scan::V},
          {za::Keyboard::Scan::B},
          {za::Keyboard::Scan::N},
          {za::Keyboard::Scan::M},
          {za::Keyboard::Scan::Comma},
          {za::Keyboard::Scan::Period},
          {za::Keyboard::Scan::Slash},
          {za::Keyboard::Scan::RShift, {2.75, 1}, 1.5},
          {za::Keyboard::Scan::Up, 1.5},
          {za::Keyboard::Scan::Numpad1},
          {za::Keyboard::Scan::Numpad2},
          {za::Keyboard::Scan::Numpad3},
          {za::Keyboard::Scan::NumpadEnter, {1, 2}}}},
        {{{za::Keyboard::Scan::LControl, {1.5, 1}},
          {za::Keyboard::Scan::LSystem, {1.25, 1}},
          {za::Keyboard::Scan::LAlt, {1.5, 1}},
          {za::Keyboard::Scan::Space, {5.75, 1}},
          {za::Keyboard::Scan::RAlt, {1.25, 1}},
          {za::Keyboard::Scan::RSystem, {1.25, 1}},
          {za::Keyboard::Scan::Menu, {1.25, 1}},
          {za::Keyboard::Scan::RControl, {1.25, 1}, 0.5},
          {za::Keyboard::Scan::Left},
          {za::Keyboard::Scan::Down},
          {za::Keyboard::Scan::Right, 0.5},
          {za::Keyboard::Scan::Numpad0, {2, 1}},
          {za::Keyboard::Scan::NumpadDecimal}},
         0.5},
        {{{za::Keyboard::Scan::F13},
          {za::Keyboard::Scan::F14},
          {za::Keyboard::Scan::F15},
          {za::Keyboard::Scan::F16},
          {za::Keyboard::Scan::F17},
          {za::Keyboard::Scan::F18},
          {za::Keyboard::Scan::F19},
          {za::Keyboard::Scan::F20},
          {za::Keyboard::Scan::F21},
          {za::Keyboard::Scan::F22},
          {za::Keyboard::Scan::F23},
          {za::Keyboard::Scan::F24}}},
        {{{za::Keyboard::Scan::Application},
          {za::Keyboard::Scan::Execute},
          {za::Keyboard::Scan::ModeChange},
          {za::Keyboard::Scan::Help},
          {za::Keyboard::Scan::Select},
          {za::Keyboard::Scan::Redo},
          {za::Keyboard::Scan::Undo},
          {za::Keyboard::Scan::Cut},
          {za::Keyboard::Scan::Copy},
          {za::Keyboard::Scan::Paste},
          {za::Keyboard::Scan::VolumeMute},
          {za::Keyboard::Scan::VolumeUp},
          {za::Keyboard::Scan::VolumeDown},
          {za::Keyboard::Scan::MediaPlayPause},
          {za::Keyboard::Scan::MediaStop},
          {za::Keyboard::Scan::MediaNextTrack},
          {za::Keyboard::Scan::MediaPreviousTrack}}},
        {{{za::Keyboard::Scan::Back},
          {za::Keyboard::Scan::Forward},
          {za::Keyboard::Scan::Refresh},
          {za::Keyboard::Scan::Stop},
          {za::Keyboard::Scan::Search},
          {za::Keyboard::Scan::Favorites},
          {za::Keyboard::Scan::HomePage},
          {za::Keyboard::Scan::LaunchMediaSelect}}},
    }};

    za::Vector<za::Vertex>                        m_triangles{za::Keyboard::ScancodeCount * 6};
    za::Vector<za::Text>                          m_labels;
    za::Array<float, za::Keyboard::ScancodeCount> m_moveFactors{};
};


////////////////////////////////////////////////////////////
// Text with fading opacity outline
////////////////////////////////////////////////////////////
class ShinyText : public za::Text
{
public:
    using za::Text::Text;

    // Start the outline animation
    void shine(za::Color color = za::Color::Yellow)
    {
        setOutlineColor(color);
        m_remaining = duration;
    }

    // Fade out ouline
    void update(za::Time frameTime)
    {
        const float ratio = m_remaining / duration;
        const float alpha = za::max(0.f, ratio * (2.f - ratio)) * 0.5f;

        za::Color color = getOutlineColor();
        color.a         = static_cast<za::U8>(255 * alpha);
        setOutlineColor(color);

        if (m_remaining > za::Time{})
            m_remaining -= frameTime;
    }

private:
    static constexpr za::Time duration = za::milliseconds(150);
    za::Time                  m_remaining;
};


////////////////////////////////////////////////////////////
// Utilities to create text objets
////////////////////////////////////////////////////////////

constexpr unsigned int textSize = 18u;
constexpr unsigned int space    = 2u;
constexpr unsigned int lineSize = textSize + space;

float getSpacingFactor(const za::Font& font)
{
    return static_cast<float>(lineSize) / font.getLineSpacing(textSize);
}

ShinyText makeShinyText(const za::Font& font, const za::Utf8String& string, za::Vec2f position)
{
    ShinyText text(font, {.string = string, .characterSize = textSize});
    text.setLineSpacing(getSpacingFactor(font));
    text.setOutlineThickness(2.f);
    text.position = position;

    return text;
}

za::Text makeText(const za::Font& font, const za::Utf8String& string, za::Vec2f position)
{
    za::Text text(font, {.string = string, .characterSize = textSize});
    text.setLineSpacing(getSpacingFactor(font));
    text.position = position;

    return text;
}


////////////////////////////////////////////////////////////
// Utilities to describe keyboard events
////////////////////////////////////////////////////////////

template <typename KeyEventType>
bool somethingIsOdd(const KeyEventType& keyEvent)
{
    return keyEvent.code == za::Keyboard::Key::Unknown || keyEvent.scancode == za::Keyboard::Scan::Unknown ||
           za::Keyboard::getDescription(keyEvent.scancode) == "" ||
           za::Keyboard::localize(keyEvent.scancode) != keyEvent.code ||
           za::Keyboard::delocalize(keyEvent.code) != keyEvent.scancode;
}

// Append information to string about a keyboard event
template <typename KeyEventType>
za::Utf8String keyEventDescription(za::Utf8String text, const KeyEventType& keyEvent)
{
    text += "\n\n";
    text += keyIdentifier(keyEvent.code);
    text += "\n";
    text += scancodeIdentifier(keyEvent.scancode);
    if (somethingIsOdd(keyEvent))
    {
        text += "\nLocalized:\t";
        text += keyIdentifier(za::Keyboard::localize(keyEvent.scancode));
        text += "\nDelocalized:\t";
        text += scancodeIdentifier(za::Keyboard::delocalize(keyEvent.code));
    }

    return text;
}

// Make a string describing a text event
za::Utf8String textEventDescription(const za::Event::TextEntered& textEntered)
{
    za::Utf8String text = "Text Entered\n\n";
    text += textEntered.unicode;
    text += "\nU+";

    (void)za::fmtTo(text, "{:0>4x}", static_cast<za::U32>(textEntered.unicode));

    return text;
}

} // namespace


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Create an audio context and get the default playback device
    auto               audioContext = za::AudioContext::create().value();
    za::PlaybackDevice playbackDevice{za::AudioContext::getDefaultPlaybackDeviceHandle().value()};

    // Create the graphics context
    auto graphicsContext = za::GraphicsContext::create().value();

    // Create the main window
    constexpr za::Vec2f windowSize{1280.f, 720.f};

    auto window = makeDPIScaledRenderWindow(
                      {
                          .size           = windowSize.toVec2u(),
                          .title          = "Keyboard",
                          .resizable      = true,
                          .vsync          = true,
                          .frametimeLimit = 25u,
                      })
                      .value();

    auto windowView = computeAspectRatioAwareView(window.getSize().toVec2f(), windowSize);

    // Load sound buffers
    const auto errorSoundBuffer    = za::SoundBuffer::loadFromFile(resourcesDir() / "error_005.ogg").value();
    const auto pressedSoundBuffer  = za::SoundBuffer::loadFromFile(resourcesDir() / "mouseclick1.ogg").value();
    const auto releasedSoundBuffer = za::SoundBuffer::loadFromFile(resourcesDir() / "mouserelease1.ogg").value();

    // Create sound objects to play them upon keyboard events
    za::Sound errorSound(playbackDevice, errorSoundBuffer);
    za::Sound pressedSound(playbackDevice, pressedSoundBuffer);
    za::Sound releasedSound(playbackDevice, releasedSoundBuffer);

    // Open the font used for all texts
    const auto font = za::Font::openFromFile(resourcesDir() / "Tuffy.ttf").value();

    // Create object to display all scancodes descriptions, related events and real-time state
    KeyboardView keyboardView(font);
    keyboardView.position = {16.f, 16.f};

    // Create text to display information about keyboard events and key codes real-time state
    ShinyText keyPressedText(makeShinyText(font, "Key Pressed", {16, 575}));
    ShinyText keyReleasedText(makeShinyText(font, "Key Released", {300, 575}));
    ShinyText textEnteredText(makeShinyText(font, "Text Entered", {600, 575}));
    za::Text  keyPressedCheckText(makeText(font, "", {900, 575}));

    za::Clock clock;
    while (true)
    {
        // Handle events
        while (za::Optional event = window.pollEvent())
        {
            // Window closed: exit
            if (za::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return 0;

            if (handleAspectRatioAwareResize(*event, windowSize, windowView))
                continue;

            // Key events: update text and play sound
            if (const auto* keyPressed = event->getIf<za::Event::KeyPressed>())
            {
                keyPressedText.setString(keyEventDescription("Key Pressed", *keyPressed));
                if (somethingIsOdd(*keyPressed))
                {
                    keyPressedText.shine(za::Color::Red);
                    errorSound.play();
                }
                else
                {
                    keyPressedText.shine(za::Color::Green);
                    pressedSound.play();
                }
            }
            if (const auto* keyReleased = event->getIf<za::Event::KeyReleased>())
            {
                keyReleasedText.setString(keyEventDescription("Key Released", *keyReleased));
                if (somethingIsOdd(*keyReleased))
                {
                    keyReleasedText.shine(za::Color::Red);
                    errorSound.play();
                }
                else
                {
                    keyReleasedText.shine(za::Color::Green);
                    releasedSound.play();
                }
            }
            if (const auto* textEntered = event->getIf<za::Event::TextEntered>())
            {
                textEnteredText.setString(textEventDescription(*textEntered));
                textEnteredText.shine();
            }

            // Let the KeyboardView process the event
            keyboardView.handle(*event);
        }

        // Update animations and displayed keyboard real-time state
        const za::Time frameTime = clock.restart();
        keyboardView.update(frameTime);
        keyPressedText.update(frameTime);
        keyReleasedText.update(frameTime);
        textEnteredText.update(frameTime);
        {
            za::Utf8String text = "isKeyPressed(za::Keyboard::Key)\n\n";
            for (za::SizeT keyIndex = 0u; keyIndex < za::Keyboard::KeyCount; ++keyIndex)
            {
                const auto key = static_cast<za::Keyboard::Key>(keyIndex);
                if (za::Keyboard::isKeyPressed(key))
                {
                    text += keyIdentifier(key);
                    text += "\n";
                }
            }
            keyPressedCheckText.setString(text);
        }

        // Render frame
        window.clear();
        window.withRenderStates({.view = windowView}).drawAll(keyboardView, keyPressedText, keyReleasedText, textEnteredText);
        window.display();
    }

    return 0;
}
