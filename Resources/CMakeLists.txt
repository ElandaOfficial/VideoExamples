file(GLOB_RECURSE asset_files "${CMAKE_CURRENT_LIST_DIR}/*.*")

juce_add_binary_data(AppResources
    HEADER_NAME Assets.h
    NAMESPACE   Assets
    SOURCES     ${asset_files})
