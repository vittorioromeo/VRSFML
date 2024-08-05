#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/ImageUtils.hpp>

// Other 1st party headers
#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/Path.hpp>

#include <Doctest.hpp>

#include <GraphicsUtil.hpp>

#include <type_traits>

TEST_CASE("[Graphics] sf::Image")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_default_constructible_v<sf::Image>);
        STATIC_CHECK(std::is_copy_constructible_v<sf::Image>);
        STATIC_CHECK(std::is_copy_assignable_v<sf::Image>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::Image>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::Image>);
    }

    SECTION("Construction")
    {
        SECTION("Invalid size")
        {
            CHECK(!sf::Image::create({10, 0}, sf::Color::Magenta).hasValue());
            CHECK(!sf::Image::create({0, 10}, sf::Color::Magenta).hasValue());
        }

        SECTION("Vector2 constructor")
        {
            const auto image = sf::Image::create(sf::Vector2u{10, 10}).value();
            CHECK(image.getSize() == sf::Vector2u{10, 10});
            CHECK(image.getPixelsPtr() != nullptr);

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    CHECK(image.getPixel(sf::Vector2u{i, j}) == sf::Color::Black);
                }
            }
        }

        SECTION("Vector2 and color constructor")
        {
            const auto image = sf::Image::create(sf::Vector2u{10, 10}, sf::Color::Red).value();
            CHECK(image.getSize() == sf::Vector2u{10, 10});
            CHECK(image.getPixelsPtr() != nullptr);

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    CHECK(image.getPixel(sf::Vector2u{i, j}) == sf::Color::Red);
                }
            }
        }

        SECTION("Vector2 and std::uint8_t* constructor")
        {
            // 10 x 10, with 4 color channels array
            std::uint8_t pixels[400]{};
            for (std::size_t i = 0; i < 400; i += 4)
            {
                pixels[i]     = 255; // r
                pixels[i + 1] = 0;   // g
                pixels[i + 2] = 0;   // b
                pixels[i + 3] = 255; // a
            }

            const auto image = sf::Image::create(sf::Vector2u{10, 10}, pixels).value();
            CHECK(image.getSize() == sf::Vector2u{10, 10});
            CHECK(image.getPixelsPtr() != nullptr);

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    CHECK(image.getPixel(sf::Vector2u{i, j}) == sf::Color::Red);
                }
            }
        }
    }

    SECTION("loadFromFile()")
    {
        SECTION("Invalid file")
        {
            CHECK(!sf::Image::loadFromFile(".").hasValue());
            CHECK(!sf::Image::loadFromFile("this/does/not/exist.jpg").hasValue());
        }

        SECTION("Successful load")
        {
            sf::base::Optional<sf::Image> image;

            SECTION("bmp")
            {
                image = sf::Image::loadFromFile("Graphics/sfml-logo-big.bmp");
                REQUIRE(image.hasValue());
                CHECK(image->getPixel({0, 0}) == sf::Color::White);
                CHECK(image->getPixel({200, 150}) == sf::Color(144, 208, 62));
            }

            SECTION("png")
            {
                image = sf::Image::loadFromFile("Graphics/sfml-logo-big.png");
                REQUIRE(image.hasValue());
                CHECK(image->getPixel({0, 0}) == sf::Color(255, 255, 255, 0));
                CHECK(image->getPixel({200, 150}) == sf::Color(144, 208, 62));
            }

            SECTION("jpg")
            {
                image = sf::Image::loadFromFile("Graphics/sfml-logo-big.jpg");
                REQUIRE(image.hasValue());
                CHECK(image->getPixel({0, 0}) == sf::Color::White);
                CHECK(image->getPixel({200, 150}) == sf::Color(144, 208, 62));
            }

            SECTION("gif")
            {
                image = sf::Image::loadFromFile("Graphics/sfml-logo-big.gif");
                REQUIRE(image.hasValue());
                CHECK(image->getPixel({0, 0}) == sf::Color::White);
                CHECK(image->getPixel({200, 150}) == sf::Color(146, 210, 62));
            }

            SECTION("psd")
            {
                image = sf::Image::loadFromFile("Graphics/sfml-logo-big.psd");
                REQUIRE(image.hasValue());
                CHECK(image->getPixel({0, 0}) == sf::Color::White);
                CHECK(image->getPixel({200, 150}) == sf::Color(144, 208, 62));
            }

            CHECK(image->getSize() == sf::Vector2u{1001, 304});
            CHECK(image->getPixelsPtr() != nullptr);
        }
    }

    SECTION("loadFromMemory()")
    {
        SECTION("Invalid pointer")
        {
            CHECK(!sf::Image::loadFromMemory(nullptr, 1).hasValue());
        }

        SECTION("Invalid size")
        {
            const std::byte testByte{0xAB};
            CHECK(!sf::Image::loadFromMemory(&testByte, 0).hasValue());
        }

        SECTION("Failed load")
        {
            std::vector<std::uint8_t> memory;

            SECTION("Empty")
            {
                memory.clear();
            }

            SECTION("Junk data")
            {
                memory = {1, 2, 3, 4};
            }

            CHECK(!sf::Image::loadFromMemory(memory.data(), memory.size()).hasValue());
        }

        SECTION("Successful load")
        {
            const auto memory = sf::ImageUtils::saveToMemory(sf::Image::create({24, 24}, sf::Color::Green).value(),
                                                             sf::ImageUtils::SaveFormat::PNG);

            const auto image = sf::Image::loadFromMemory(memory.data(), memory.size()).value();
            CHECK(image.getSize() == sf::Vector2u{24, 24});
            CHECK(image.getPixelsPtr() != nullptr);
            CHECK(image.getPixel({0, 0}) == sf::Color::Green);
            CHECK(image.getPixel({23, 23}) == sf::Color::Green);
        }
    }

    SECTION("loadFromStream()")
    {
        auto       stream = sf::FileInputStream::open("Graphics/sfml-logo-big.png").value();
        const auto image  = sf::Image::loadFromStream(stream).value();
        CHECK(image.getSize() == sf::Vector2u{1001, 304});
        CHECK(image.getPixelsPtr() != nullptr);
        CHECK(image.getPixel({0, 0}) == sf::Color(255, 255, 255, 0));
        CHECK(image.getPixel({200, 150}) == sf::Color(144, 208, 62));
    }

    SECTION("saveToFile()")
    {
        const auto image = sf::Image::create({256, 256}, sf::Color::Magenta).value();

        SECTION("No extension")
        {
            CHECK(!sf::ImageUtils::saveToFile(image, "wheresmyextension"));
            CHECK(!sf::ImageUtils::saveToFile(image, "pls/add/extension"));
        }

        SECTION("Invalid extension")
        {
            CHECK(!sf::ImageUtils::saveToFile(image, "test.ps"));
            CHECK(!sf::ImageUtils::saveToFile(image, "test.foo"));
        }

        SECTION("Successful save")
        {
            auto filename = sf::Path::tempDirectoryPath();

            SECTION("To .bmp")
            {
                filename /= "test.bmp";
                CHECK(sf::ImageUtils::saveToFile(image, filename));
            }

            SECTION("To .tga")
            {
                filename /= "test.tga";
                CHECK(sf::ImageUtils::saveToFile(image, filename));
            }

            SECTION("To .png")
            {
                filename /= "test.png";
                CHECK(sf::ImageUtils::saveToFile(image, filename));
            }

            // Cannot test JPEG encoding due to it triggering UB in stbiw__jpg_writeBits

            const auto loadedImage = sf::Image::loadFromFile(filename).value();
            CHECK(loadedImage.getSize() == sf::Vector2u{256, 256});
            CHECK(loadedImage.getPixelsPtr() != nullptr);

            CHECK(filename.remove());
        }
    }

    SECTION("saveToMemory()")
    {
        const auto image = sf::Image::create({16, 16}, sf::Color::Magenta).value();

        SECTION("Successful save")
        {
            std::vector<std::uint8_t> output;

            SECTION("To bmp")
            {
                output = sf::ImageUtils::saveToMemory(image, sf::ImageUtils::SaveFormat::BMP);
                REQUIRE(output.size() == 1146);
                CHECK(output[0] == 66);
                CHECK(output[1] == 77);
                CHECK(output[2] == 122);
                CHECK(output[3] == 4);
                CHECK(output[1000] == 255);
                CHECK(output[1001] == 255);
                CHECK(output[1002] == 255);
                CHECK(output[1003] == 0);
            }

            SECTION("To tga")
            {
                output = sf::ImageUtils::saveToMemory(image, sf::ImageUtils::SaveFormat::TGA);
                REQUIRE(output.size() == 98);
                CHECK(output[0] == 0);
                CHECK(output[1] == 0);
                CHECK(output[2] == 10);
                CHECK(output[3] == 0);
            }

            SECTION("To png")
            {
                output = sf::ImageUtils::saveToMemory(image, sf::ImageUtils::SaveFormat::PNG);
                REQUIRE(output.size() == 92);
                CHECK(output[0] == 137);
                CHECK(output[1] == 80);
                CHECK(output[2] == 78);
                CHECK(output[3] == 71);
            }

            // Cannot test JPEG encoding due to it triggering UB in stbiw__jpg_writeBits
        }
    }

    SECTION("Set/get pixel")
    {
        auto image = sf::Image::create(sf::Vector2u{10, 10}, sf::Color::Green).value();
        CHECK(image.getPixel(sf::Vector2u{2, 2}) == sf::Color::Green);

        image.setPixel(sf::Vector2u{2, 2}, sf::Color::Blue);
        CHECK(image.getPixel(sf::Vector2u{2, 2}) == sf::Color::Blue);
    }

    SECTION("Copy from Image")
    {
        SECTION("Copy (Image, Vector2u)")
        {
            const auto image1 = sf::Image::create(sf::Vector2u{10, 10}, sf::Color::Blue).value();
            auto       image2 = sf::Image::create(sf::Vector2u{10, 10}).value();
            CHECK(image2.copy(image1, sf::Vector2u{0, 0}));

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    CHECK(image1.getPixel(sf::Vector2u{i, j}) == image2.getPixel(sf::Vector2u{i, j}));
                }
            }
        }

        SECTION("Copy (Image, Vector2u, IntRect)")
        {
            const auto image1 = sf::Image::create(sf::Vector2u{5, 5}, sf::Color::Blue).value();
            auto       image2 = sf::Image::create(sf::Vector2u{10, 10}).value();
            CHECK(image2.copy(image1, sf::Vector2u{0, 0}, sf::IntRect(sf::Vector2i{0, 0}, sf::Vector2i{5, 5})));

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    if (i <= 4 && j <= 4)
                        CHECK(image2.getPixel(sf::Vector2u{i, j}) == sf::Color::Blue);
                    else
                        CHECK(image2.getPixel(sf::Vector2u{i, j}) == sf::Color::Black);
                }
            }
        }

        SECTION("Copy (Image, Vector2u, IntRect, bool)")
        {
            const sf::Color dest(255, 0, 0, 255);
            const sf::Color source(5, 255, 78, 232);

            // Create the composited color for via the alpha composite over operation
            const auto a = static_cast<std::uint8_t>(source.a + (dest.a * (255 - source.a)) / 255);
            const auto r = static_cast<std::uint8_t>(
                ((source.r * source.a) + ((dest.r * dest.a) * (255 - source.a)) / 255) / a);
            const auto g = static_cast<std::uint8_t>(
                ((source.g * source.a) + ((dest.g * dest.a) * (255 - source.a)) / 255) / a);
            const auto b = static_cast<std::uint8_t>(
                ((source.b * source.a) + ((dest.b * dest.a) * (255 - source.a)) / 255) / a);
            const sf::Color composite(r, g, b, a);

            auto       image1 = sf::Image::create(sf::Vector2u{10, 10}, dest).value();
            const auto image2 = sf::Image::create(sf::Vector2u{10, 10}, source).value();
            CHECK(image1.copy(image2, sf::Vector2u{0, 0}, sf::IntRect(sf::Vector2i{0, 0}, sf::Vector2i{10, 10}), true));

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    CHECK(image1.getPixel(sf::Vector2u{i, j}) == composite);
                }
            }
        }

        SECTION("Copy (Out of bounds sourceRect)")
        {
            const auto image1 = sf::Image::create(sf::Vector2u{5, 5}, sf::Color::Blue).value();
            auto       image2 = sf::Image::create(sf::Vector2u{10, 10}, sf::Color::Red).value();
            CHECK(!image2.copy(image1, sf::Vector2u{0, 0}, sf::IntRect(sf::Vector2i{5, 5}, sf::Vector2i{9, 9})));

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    CHECK(image2.getPixel(sf::Vector2u{i, j}) == sf::Color::Red);
                }
            }
        }
    }

    SECTION("Create mask from color")
    {
        SECTION("createMaskFromColor(Color)")
        {
            auto image = sf::Image::create(sf::Vector2u{10, 10}, sf::Color::Blue).value();
            image.createMaskFromColor(sf::Color::Blue);

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    CHECK(image.getPixel(sf::Vector2u{i, j}) == sf::Color(0, 0, 255, 0));
                }
            }
        }

        SECTION("createMaskFromColor(Color, std::uint8_t)")
        {
            auto image = sf::Image::create(sf::Vector2u{10, 10}, sf::Color::Blue).value();
            image.createMaskFromColor(sf::Color::Blue, 100);

            for (std::uint32_t i = 0; i < 10; ++i)
            {
                for (std::uint32_t j = 0; j < 10; ++j)
                {
                    CHECK(image.getPixel(sf::Vector2u{i, j}) == sf::Color(0, 0, 255, 100));
                }
            }
        }
    }

    SECTION("Flip horizontally")
    {
        auto image = sf::Image::create(sf::Vector2u{10, 10}, sf::Color::Red).value();
        image.setPixel(sf::Vector2u{0, 0}, sf::Color::Green);
        image.flipHorizontally();

        CHECK(image.getPixel(sf::Vector2u{9, 0}) == sf::Color::Green);
    }

    SECTION("Flip vertically")
    {
        auto image = sf::Image::create(sf::Vector2u{10, 10}, sf::Color::Red).value();
        image.setPixel(sf::Vector2u{0, 0}, sf::Color::Green);
        image.flipVertically();

        CHECK(image.getPixel(sf::Vector2u{0, 9}) == sf::Color::Green);
    }
}
