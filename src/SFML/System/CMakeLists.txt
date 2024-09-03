set(BASE_INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Base)
set(BASE_SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Base)

# all source files
set(SRC_BASE
    # headers
    ${BASE_INCROOT}/Algorithm.hpp
    ${BASE_INCROOT}/Assert.hpp
    ${BASE_INCROOT}/EnumArray.hpp
    ${BASE_INCROOT}/EnumClassBitwiseOps.hpp
    ${BASE_INCROOT}/FixedFunction.hpp
    ${BASE_INCROOT}/IndexSequence.hpp
    ${BASE_INCROOT}/InPlacePImpl.hpp
    ${BASE_INCROOT}/Launder.hpp
    ${BASE_INCROOT}/Macros.hpp
    ${BASE_INCROOT}/MakeIndexSequence.hpp
    ${BASE_INCROOT}/Math/Acos.hpp
    ${BASE_INCROOT}/Math/Asin.hpp
    ${BASE_INCROOT}/Math/Atan.hpp
    ${BASE_INCROOT}/Math/Atan2.hpp
    ${BASE_INCROOT}/Math/Ceil.hpp
    ${BASE_INCROOT}/Math/Cos.hpp
    ${BASE_INCROOT}/Math/Cosh.hpp
    ${BASE_INCROOT}/Math/Exp.hpp
    ${BASE_INCROOT}/Math/Fabs.hpp
    ${BASE_INCROOT}/Math/Floor.hpp
    ${BASE_INCROOT}/Math/Fmod.hpp
    ${BASE_INCROOT}/Math/Frexp.hpp
    ${BASE_INCROOT}/Math/Impl.hpp
    ${BASE_INCROOT}/Math/ImplBuiltinWrapper.hpp
    ${BASE_INCROOT}/Math/ImplStdForwarder.hpp
    ${BASE_INCROOT}/Math/ImplUndef.hpp
    ${BASE_INCROOT}/Math/Ldexp.hpp
    ${BASE_INCROOT}/Math/Log.hpp
    ${BASE_INCROOT}/Math/Log10.hpp
    ${BASE_INCROOT}/Math/Lround.hpp
    ${BASE_INCROOT}/Math/Sin.hpp
    ${BASE_INCROOT}/Math/Sqrt.hpp
    ${BASE_INCROOT}/MaxAlignT.hpp
    ${BASE_INCROOT}/Optional.hpp
    ${BASE_INCROOT}/OverloadSet.hpp
    ${BASE_INCROOT}/PassKey.hpp
    ${BASE_INCROOT}/PlacementNew.hpp
    ${BASE_INCROOT}/SizeT.hpp
    ${BASE_INCROOT}/Traits/HasVirtualDestructor.hpp
    ${BASE_INCROOT}/Traits/IsAggregate.hpp
    ${BASE_INCROOT}/Traits/IsBaseOf.hpp
    ${BASE_INCROOT}/Traits/IsConstructible.hpp
    ${BASE_INCROOT}/Traits/IsConvertible.hpp
    ${BASE_INCROOT}/Traits/IsCopyAssignable.hpp
    ${BASE_INCROOT}/Traits/IsCopyConstructible.hpp
    ${BASE_INCROOT}/Traits/IsDefaultConstructible.hpp
    ${BASE_INCROOT}/Traits/IsEmpty.hpp
    ${BASE_INCROOT}/Traits/IsEnum.hpp
    ${BASE_INCROOT}/Traits/IsFloatingPoint.hpp
    ${BASE_INCROOT}/Traits/IsMoveAssignable.hpp
    ${BASE_INCROOT}/Traits/IsMoveConstructible.hpp
    ${BASE_INCROOT}/Traits/IsNothrowMoveAssignable.hpp
    ${BASE_INCROOT}/Traits/IsNothrowMoveConstructible.hpp
    ${BASE_INCROOT}/Traits/IsNothrowSwappable.hpp
    ${BASE_INCROOT}/Traits/IsReference.hpp
    ${BASE_INCROOT}/Traits/IsRvalueReference.hpp
    ${BASE_INCROOT}/Traits/IsSame.hpp
    ${BASE_INCROOT}/Traits/IsStandardLayout.hpp
    ${BASE_INCROOT}/Traits/IsTrivial.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyAssignable.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyCopyable.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyCopyAssignable.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyCopyConstructible.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyDestructible.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyMoveAssignable.hpp
    ${BASE_INCROOT}/Traits/IsTriviallyMoveConstructible.hpp
    ${BASE_INCROOT}/Traits/RemoveCVRef.hpp
    ${BASE_INCROOT}/Traits/RemoveReference.hpp
    ${BASE_INCROOT}/Traits/UnderlyingType.hpp
    ${BASE_INCROOT}/TypePackElement.hpp
    ${BASE_INCROOT}/UniquePtr.hpp
    ${BASE_INCROOT}/Variant.hpp

    # sources
    ${BASE_SRCROOT}/Assert.cpp
    ${BASE_SRCROOT}/Optional.cpp
    ${BASE_SRCROOT}/StackTrace.cpp
    ${BASE_SRCROOT}/StackTrace.hpp
)





set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/System)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/System)

# all source files
set(SRC
    ${INCROOT}/Angle.hpp
    ${INCROOT}/Angle.inl
    ${INCROOT}/Clock.hpp
    ${INCROOT}/Err.hpp
    ${INCROOT}/Export.hpp
    ${INCROOT}/FileInputStream.hpp
    ${INCROOT}/InputStream.hpp
    ${INCROOT}/LifetimeDependant.hpp
    ${INCROOT}/LifetimeDependee.hpp
    ${INCROOT}/MemoryInputStream.hpp
    ${INCROOT}/NativeActivity.hpp
    ${INCROOT}/Path.hpp
    ${INCROOT}/Rect.hpp
    ${INCROOT}/Rect.inl
    ${INCROOT}/RectPacker.hpp
    ${INCROOT}/RectUtils.hpp
    ${INCROOT}/RectUtils.inl
    ${INCROOT}/Sleep.hpp
    ${INCROOT}/String.hpp
    ${INCROOT}/StringUtfUtils.hpp
    ${INCROOT}/StringUtfUtils.inl
    ${INCROOT}/SuspendAwareClock.hpp
    ${INCROOT}/Time.hpp
    ${INCROOT}/Time.inl
    ${INCROOT}/TimeChronoUtil.hpp
    ${INCROOT}/TimeChronoUtil.inl
    ${INCROOT}/Utf.hpp
    ${INCROOT}/Utf.inl
    ${INCROOT}/Vector2.hpp
    ${INCROOT}/Vector2.inl
    ${INCROOT}/Vector3.hpp
    ${INCROOT}/Vector3.inl
    ${SRCROOT}/Clock.cpp
    ${SRCROOT}/Err.cpp
    ${SRCROOT}/FileInputStream.cpp
    ${SRCROOT}/LifetimeDependant.cpp
    ${SRCROOT}/LifetimeDependee.cpp
    ${SRCROOT}/MemoryInputStream.cpp
    ${SRCROOT}/Path.cpp
    ${SRCROOT}/PathUtils.hpp
    ${SRCROOT}/Rect.cpp
    ${SRCROOT}/RectPacker.cpp
    ${SRCROOT}/Sleep.cpp
    ${SRCROOT}/String.cpp
    ${SRCROOT}/StringUtils.cpp
    ${SRCROOT}/StringUtils.hpp
    ${SRCROOT}/Utils.hpp
    ${SRCROOT}/Vector2.cpp
    ${SRCROOT}/Vector3.cpp
)

list(APPEND SRC ${SRC_BASE})

if(SFML_ENABLE_PCH)
    list(APPEND SRC ${SRCROOT}/PCH.cpp)
endif()

source_group("" FILES ${SRC})

# add platform specific sources
if(SFML_OS_WINDOWS)
    set(PLATFORM_SRC
        ${SRCROOT}/Win32/SleepImpl.cpp
        ${SRCROOT}/Win32/SleepImpl.hpp
    )
    source_group("windows" FILES ${PLATFORM_SRC})
else()
    set(PLATFORM_SRC
        ${SRCROOT}/Unix/SleepImpl.cpp
        ${SRCROOT}/Unix/SleepImpl.hpp
    )

    if(SFML_OS_ANDROID)
        list(APPEND PLATFORM_SRC
            ${SRCROOT}/Android/Activity.hpp
            ${SRCROOT}/Android/Activity.cpp
            ${SRCROOT}/Android/NativeActivity.cpp
            ${SRCROOT}/Android/ResourceStream.cpp
            ${SRCROOT}/Android/ResourceStream.cpp
            ${SRCROOT}/Android/SuspendAwareClock.cpp
        )
    endif()

    source_group("unix" FILES ${PLATFORM_SRC})
endif()

find_package(Threads REQUIRED)

# define the sfml-system target
sfml_add_library(System
                 SOURCES ${SRC} ${PLATFORM_SRC})

# stb_rect_pack sources
target_include_directories(sfml-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/stb_rect_pack")

# enable precompiled headers
if (SFML_ENABLE_PCH)
    message(VERBOSE "enabling PCH for SFML library 'sfml-system' (reused as the PCH for other SFML libraries)")
    target_precompile_headers(sfml-system PRIVATE ${PROJECT_SOURCE_DIR}/src/SFML/PCH.hpp)
endif()

if(SFML_OS_ANDROID)
    # glad sources
    target_include_directories(sfml-system SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")
endif()

target_link_libraries(sfml-system PRIVATE Threads::Threads)

if(SFML_OS_LINUX)
    target_link_libraries(sfml-system PRIVATE rt)
elseif(SFML_OS_WINDOWS)
    target_link_libraries(sfml-system PRIVATE winmm)
elseif(SFML_OS_ANDROID)
    target_link_libraries(sfml-system PRIVATE android log)
endif()

if(SFML_ENABLE_STACK_TRACES)
    target_link_libraries(sfml-system PUBLIC cpptrace::cpptrace)
endif()
