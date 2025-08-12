# CMake version and project
cmake_minimum_required(VERSION 3.10)
project(WinPal)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory for executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define source files
set(SOURCES
    main.cpp
    Core/GuiManager.cpp
    Core/HotkeyManager.cpp
    Commands/CommandManager.cpp
    Commands/ExecutionHistory.cpp
    Plugins/SystemSettings/SettingsCommand.cpp
    Plugins/FileTools/OpenFileExplorerCommand.cpp
    Plugins/FileTools/OpenDownloadsCommand.cpp
    Plugins/FileTools/OpenDesktopCommand.cpp
    Plugins/ApplicationLauncher/LaunchCalculatorCommand.cpp
    Plugins/ApplicationLauncher/LaunchNotepadCommand.cpp
    Plugins/ApplicationLauncher/LaunchTaskManagerCommand.cpp
    Plugins/ApplicationLauncher/GenericLaunchCommand.cpp
    Plugins/ApplicationLauncher/ApplicationFinder.cpp
    Plugins/ApplicationLauncher/RefreshApplicationsCommand.cpp
    Plugins/SystemInfo/ShowSystemInfoCommand.cpp
    Plugins/SystemInfo/ShowDiskUsageCommand.cpp
    Plugins/NetworkTools/ShowNetworkInfoCommand.cpp
    Plugins/NetworkTools/PingCommand.cpp
    Plugins/ClipboardManager/ClearClipboardCommand.cpp
    Plugins/DeveloperTools/OpenGitBashCommand.cpp
    Plugins/DeveloperTools/OpenPowerShellCommand.cpp
    Plugins/ProcessTools/EnterProcessModeCommand.cpp
    Plugins/ProcessTools/TerminateProcessCommand.cpp
    Plugins/ProcessTools/OpenProcessPathCommand.cpp
)

# Define header files for better IDE support
set(HEADERS
    Core/GuiManager.h
    Core/HotkeyManager.h
    Commands/CommandManager.h
    Commands/ExecutionHistory.h
    Commands/ICommand.h
    Plugins/SystemSettings/SettingsCommand.h
    Plugins/FileTools/OpenFileExplorerCommand.h
    Plugins/FileTools/OpenDownloadsCommand.h
    Plugins/FileTools/OpenDesktopCommand.h
    Plugins/ApplicationLauncher/LaunchCalculatorCommand.h
    Plugins/ApplicationLauncher/LaunchNotepadCommand.h
    Plugins/ApplicationLauncher/LaunchTaskManagerCommand.h
    Plugins/ApplicationLauncher/GenericLaunchCommand.h
    Plugins/ApplicationLauncher/ApplicationFinder.h
    Plugins/ApplicationLauncher/RefreshApplicationsCommand.h
    Plugins/SystemInfo/ShowSystemInfoCommand.h
    Plugins/SystemInfo/ShowDiskUsageCommand.h
    Plugins/NetworkTools/ShowNetworkInfoCommand.h
    Plugins/NetworkTools/PingCommand.h
    Plugins/ClipboardManager/ClearClipboardCommand.h
    Plugins/DeveloperTools/OpenGitBashCommand.h
    Plugins/DeveloperTools/OpenPowerShellCommand.h
    Plugins/ProcessTools/EnterProcessModeCommand.h
    Plugins/ProcessTools/TerminateProcessCommand.h
    Plugins/ProcessTools/OpenProcessPathCommand.h
)

# Create the executable
add_executable(WinPal ${SOURCES} ${HEADERS})

# Link necessary libraries
target_link_libraries(WinPal
    user32
    gdi32
    shell32
    ole32
    oleaut32
    uuid
    comctl32
    comdlg32
    advapi32
    version
    dwmapi
    gdiplus
    shlwapi
    wbemuuid
)

# Compiler-specific options
if(MSVC)
    # Use Unicode
    target_compile_definitions(WinPal PRIVATE UNICODE _UNICODE)
    
    # Enable modern C++ features
    target_compile_options(WinPal PRIVATE /std:c++17)
    
    # Optimization for release builds
    target_compile_options(WinPal PRIVATE "$<$<CONFIG:Release>:/O2>")
    
    # Debug information for debug builds
    target_compile_options(WinPal PRIVATE "$<$<CONFIG:Debug>:/Zi>")
endif()

# Set target properties
set_target_properties(WinPal PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "WinPal"
)

# Include directories
target_include_directories(WinPal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Ensure output directory exists
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Copy any additional files if needed
# add_custom_command(TARGET WinPal POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#     "source_file" $<TARGET_FILE_DIR:WinPal>)

message(STATUS "WinPal configured successfully")
message(STATUS "Source files: ${SOURCES}")
message(STATUS "Output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")