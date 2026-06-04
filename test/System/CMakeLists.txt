file(GLOB SYSTEM_SRC CONFIGURE_DEPENDS "*.cpp" "*.hpp" "*.inl")
zancle_add_test(test-zancle-system "${SYSTEM_SRC}" "")

target_compile_definitions(test-zancle-system PRIVATE
    EXPECTED_ZA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    EXPECTED_ZA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    EXPECTED_ZA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    EXPECTED_ZA_VERSION_IS_RELEASE=$<IF:$<BOOL:${VERSION_IS_RELEASE}>,true,false>
)

if(XCODE AND ZA_OS_IOS)
    set(SYSTEM_RESOURCE
        test.txt
        test2.txt
        test-ń.txt
        test-🐌.txt
    )
    target_sources(test-zancle-system PRIVATE ${SYSTEM_RESOURCE})
    set_source_files_properties(${SYSTEM_RESOURCE} PROPERTIES MACOSX_PACKAGE_LOCATION "System")
endif()
