set(VIDEO_EDITOR_VERSION 0.1.0)
set(VIDEO_EDITOR_NAME    "FF Video Editor")

juce_add_gui_app(VideoEditor
    VERSION         ${VIDEO_EDITOR_VERSION}
    COMPANY_NAME    ${PROJECT_COMPANY_NAME}
    PRODUCT_NAME    ${VIDEO_EDITOR_NAME}
    COMPANY_WEBSITE "https://foleysfinest.com/developer/videoengine/")

target_compile_definitions(VideoEditor PRIVATE
    PROJECT_INFO_NAME="${VIDEO_EDITOR_NAME}"
    PROJECT_INFO_VERSION="${VIDEO_EDITOR_VERSION}"
    PROJECT_INFO_COMPANY="${PROJECT_COMPANY_NAME}")

add_subdirectory(Source)

target_compile_definitions(VideoEditor PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    FF_AUDIO_ALLOW_ALLOCATIONS_IN_MEASURE_BLOCK=0)

target_link_libraries(VideoEditor PRIVATE
    # Resources
    AppResources

    # FFMPEG
    avformat
    avutil
    avcodec
    swscale
    swresample

    # External modules
    foleys::ff_meters
    foleys::foleys_video_engine
    jaut::jaut_gui

    # Juce modules
    juce::juce_gui_extra
    juce::juce_data_structures

    # Juce flags
    juce::juce_recommended_warning_flags
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags)
