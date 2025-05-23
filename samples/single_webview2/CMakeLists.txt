set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core GUI Widgets REQUIRED)

file(GLOB HEADERS LIST_DIRECTORIES false *.h)
file(GLOB SOURCES LIST_DIRECTORIES false *.cpp)
file(GLOB RES "*.rc")

add_executable(SingleWebView2 
  ${HEADERS}
  ${SOURCES}
  ${RES}
)

set_target_properties(SingleWebView2 PROPERTIES FOLDER "Samples")

target_compile_definitions(SingleWebView2 PRIVATE
  UNICODE
  _UNICODE
)

target_include_directories(SingleWebView2 PRIVATE 
  ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(SingleWebView2 PRIVATE
  Qt5::Core
  Qt5::Gui 
  Qt5::Widgets
  QWebView.WebView2
)

set_target_properties(SingleWebView2 PROPERTIES
  WIN32_EXECUTABLE TRUE
  VS_DEBUGGER_ENVIRONMENT "PATH=${Qt5_DIR}/../../../bin;%PATH%"
)

add_custom_command(TARGET SingleWebView2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../asserts" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/asserts"
)
