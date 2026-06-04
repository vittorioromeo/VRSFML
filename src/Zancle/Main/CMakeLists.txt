set(SRCROOT ${PROJECT_SOURCE_DIR}/src/Zancle/Main)

# sources
if(ZA_OS_WINDOWS)
    set(SRC ${SRCROOT}/MainWin32.cpp)
elseif(ZA_OS_IOS)
    enable_language(OBJCXX)
    set(SRC ${SRCROOT}/MainiOS.mm)
elseif(ZA_OS_ANDROID)
    set(SRC ${SRCROOT}/MainAndroid.cpp)
else()
    return()
endif()

# define the zancle-main target
zancle_add_library(Main STATIC SOURCES ${SRC})

# don't use -s suffix for zancle-main
set_target_properties(zancle-main PROPERTIES
                      DEBUG_POSTFIX -d
                      RELEASE_POSTFIX ""
                      MINSIZEREL_POSTFIX ""
                      RELWITHDEBINFO_POSTFIX "")

if(ZA_OS_ANDROID)
    # ensure that linking into shared libraries doesn't fail
    set_target_properties(zancle-main PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # glad sources
    target_include_directories(zancle-main SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")
elseif(ZA_OS_IOS)
    target_link_libraries(zancle-main PUBLIC Zancle::Window)
endif()
