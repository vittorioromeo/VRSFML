#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/TextUtils.hpp"

#include "SFML/Base/Math/Round.hpp"

#include <SheenBidi/SheenBidi.h>
#include <freetype/freetype.h>
#include <hb-ft.h>
#include <hb.h>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr hb_buffer_cluster_level_t toHBClusterGrouping(
    const sf::Text::ClusterGrouping clusterGrouping)
{
    switch (clusterGrouping)
    {
        case sf::Text::ClusterGrouping::Grapheme:
            return HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES;
        case sf::Text::ClusterGrouping::Character:
            return HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS;
        case sf::Text::ClusterGrouping::None:
            return HB_BUFFER_CLUSTER_LEVEL_CHARACTERS;
    }

    SFML_BASE_ASSERT(false);
    SFML_BASE_UNREACHABLE();
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline, gnu::const]] inline constexpr hb_direction_t toHBDirection(const sf::Text::Orientation orientation)
{
    switch (orientation)
    {
        case sf::TextOrientation::Default:
            return HB_DIRECTION_INVALID;
        case sf::Text::Orientation::TopToBottom:
            return HB_DIRECTION_TTB;
        case sf::Text::Orientation::BottomToTop:
            return HB_DIRECTION_BTT;
    }

    SFML_BASE_ASSERT(false);
    SFML_BASE_UNREACHABLE();
}

} // namespace


namespace sf::TextUtils
{
////////////////////////////////////////////////////////////
// Split string into segments with uniform text properties
void populateStringSegments(const sf::String& input, sf::base::Vector<StringSegment>& outputVec)
{
    const SBCodepointSequence codepointSequence{SBStringEncodingUTF32,
                                                static_cast<const void*>(input.getData()),
                                                input.getSize()};

    auto* const scriptLocator   = SBScriptLocatorCreate();
    auto* const algorithm       = SBAlgorithmCreate(&codepointSequence);
    SBUInteger  paragraphOffset = 0;

    while (paragraphOffset < input.getSize())
    {
        SBUInteger paragraphLength{};
        SBUInteger separatorLength{};
        SBAlgorithmGetParagraphBoundary(algorithm, paragraphOffset, static_cast<SBUInteger>(-1), &paragraphLength, &separatorLength);

        // If the paragraph contains characters besides the separator,
        // split the separator off into its own paragraph in the next iteration
        // We do this to ensure line breaks are inserted into segments last
        // after all character runs on the same line have already been inserted
        // This allows us to draw our segments in left-to-right top-to-bottom order
        if (separatorLength < paragraphLength)
            paragraphLength -= separatorLength;

        auto* const paragraph = SBAlgorithmCreateParagraph(algorithm, paragraphOffset, paragraphLength, SBLevelDefaultLTR);
        auto* const line     = SBParagraphCreateLine(paragraph, paragraphOffset, paragraphLength);
        const auto  runCount = SBLineGetRunCount(line);
        const auto* runArray = SBLineGetRunsPtr(line);

        for (SBUInteger i = 0; i < runCount; i++)
        {
            // Odd levels are RTL, even levels are LTR
            const hb_direction_t direction = (runArray[i].level % 2) ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;

            const SBCodepointSequence codepointSubsequence{SBStringEncodingUTF32,
                                                           static_cast<const void*>(input.getData() + runArray[i].offset),
                                                           runArray[i].length};

            SBScriptLocatorLoadCodepoints(scriptLocator, &codepointSubsequence);

            while (SBScriptLocatorMoveNext(scriptLocator))
            {
                const auto*       agent  = SBScriptLocatorGetAgent(scriptLocator);
                const hb_script_t script = hb_script_from_iso15924_tag(SBScriptGetUnicodeTag(agent->script));

                outputVec.emplaceBack(runArray[i].offset + agent->offset, agent->length, script, direction);
            }

            SBScriptLocatorReset(scriptLocator);
        }

        SBLineRelease(line);
        SBParagraphRelease(paragraph);

        paragraphOffset += paragraphLength;
    }

    SBAlgorithmRelease(algorithm);
    SBScriptLocatorRelease(scriptLocator);
}


////////////////////////////////////////////////////////////
// Our shape function
void populateShapedGlyphs(
    void* const                            shaper,
    const unsigned int                     characterSize,
    const sf::String&                      input,
    const sf::base::Vector<sf::base::U32>& indices,
    const int                              script,
    const sf::Text::Orientation            orientation,
    const sf::Text::ClusterGrouping        clusterGrouping,
    const float                            outlineThickness,
    const sf::base::U32                    style,
    sf::base::Vector<GlyphData>&           outputVec)
{
    SFML_BASE_ASSERT(input.getSize() == indices.size() && "Input string length does not match indices count");

    static thread_local hb_buffer_t* buffer = hb_buffer_create();

    // Clear out and add the input to the buffer
    hb_buffer_clear_contents(buffer);
    hb_buffer_pre_allocate(buffer, static_cast<unsigned int>(input.getSize()));

    // Instead of using hb_buffer_add_utf32, we have to use hb_buffer_add
    // to specify the initial cluster IDs for every character
    for (auto i = 0u; i < input.getSize(); ++i)
        hb_buffer_add(buffer, input[i], indices[i]);

    // hb_buffer_add doesn't automatically set the buffer content type so do it now
    hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);

    // Set the script and direction we detected during segmentation
    hb_buffer_set_script(buffer, static_cast<hb_script_t>(script));
    hb_buffer_set_direction(buffer, toHBDirection(orientation));

    // Try to guess the language of the text the user provided
    hb_buffer_guess_segment_properties(buffer);

    // Set the cluster level
    hb_buffer_set_cluster_level(buffer, toHBClusterGrouping(clusterGrouping));

    // Set load flags analogous to the Font implementation
    const FT_Int32 flags = outlineThickness == 0.f ? FT_LOAD_TARGET_NORMAL : FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_BITMAP;
    hb_ft_font_set_load_flags(static_cast<hb_font_t*>(shaper), flags);

    // Shape the text
    hb_shape(static_cast<hb_font_t*>(shaper), buffer, nullptr, 0);

    // Retrieve position data glyph by glyph
    const auto  glyphCount     = hb_buffer_get_length(buffer);
    const auto* glyphInfo      = hb_buffer_get_glyph_infos(buffer, nullptr);
    const auto* glyphPositions = hb_buffer_get_glyph_positions(buffer, nullptr);

    // HarfBuzz returns position data in scaled units
    // We need to convert them to pixels by dividing by the
    // current font scale factoring out the current character size
    // Also flip Y since HarfBuzz uses positive Y up coordinates
    sf::Vec2i fontScale;
    hb_font_get_scale(static_cast<hb_font_t*>(shaper), &fontScale.x, &fontScale.y);

    const sf::Vec2f divisor{static_cast<float>(fontScale.x) / static_cast<float>(characterSize),
                            static_cast<float>(fontScale.y) / -static_cast<float>(characterSize)};

    for (auto i = 0u; i < glyphCount; ++i)
    {
        GlyphData glyphData{glyphInfo[i].codepoint,
                            glyphInfo[i].cluster,
                            sf::Vec2f{SFML_BASE_MATH_ROUNDF(static_cast<float>(glyphPositions[i].x_offset) / divisor.x),
                                      SFML_BASE_MATH_ROUNDF(static_cast<float>(glyphPositions[i].y_offset) / divisor.y)},
                            sf::Vec2f{SFML_BASE_MATH_ROUNDF(static_cast<float>(glyphPositions[i].x_advance) / divisor.x),
                                      SFML_BASE_MATH_ROUNDF(static_cast<float>(glyphPositions[i].y_advance) / divisor.y)},
                            hb_buffer_get_direction(buffer)};

        // Adjust advances if we are shaping bold text
        // There doesn't seem to be a standardized way to do this so we will use 0.8f
        // since it sits in the middle ground of common word processor applications
        if ((style & static_cast<sf::base::U32>(sf::Text::Style::Bold)) != 0)
        {
            glyphData.advance.x += (glyphData.advance.x != 0.f) ? ((glyphData.advance.x >= 0.f) ? 0.8f : -0.8f) : 0.f;
            glyphData.advance.y += (glyphData.advance.y != 0.f) ? ((glyphData.advance.y >= 0.f) ? 0.8f : -0.8f) : 0.f;
        }

        outputVec.pushBack(glyphData);
    }
}

} // namespace sf::TextUtils
