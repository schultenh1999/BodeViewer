cmake_minimum_required(VERSION 3.16)
project(BodeViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport Test)

add_library(bode_core STATIC
    src/core/TransferFunction.h
    src/core/TransferFunction.cpp
    src/core/BodeCalculator.h
    src/core/BodeCalculator.cpp
)
target_include_directories(bode_core PUBLIC src/core)

add_library(qcustomplot STATIC
    external/qcustomplot/qcustomplot.h
    external/qcustomplot/qcustomplot.cpp
)
target_include_directories(qcustomplot PUBLIC external/qcustomplot)
target_link_libraries(qcustomplot PUBLIC Qt6::Widgets Qt6::PrintSupport)

add_library(bode_ui STATIC
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/BodePlotWidget.h
    src/ui/BodePlotWidget.cpp
)
target_include_directories(bode_ui PUBLIC src/ui)
target_link_libraries(bode_ui PUBLIC bode_core qcustomplot Qt6::Widgets)

add_executable(BodeViewer src/main.cpp)
target_link_libraries(BodeViewer PRIVATE bode_ui)

# Tests: ctest --test-dir build --output-on-failure
enable_testing()

add_executable(bode_core_tests tests/test_core.cpp tests/TestSupport.cpp)
target_link_libraries(bode_core_tests PRIVATE bode_core)
add_test(NAME core_tests COMMAND bode_core_tests)

add_executable(bode_ui_tests tests/test_ui.cpp tests/TestSupport.cpp)
target_link_libraries(bode_ui_tests PRIVATE bode_ui Qt6::Test)
add_test(NAME ui_tests COMMAND bode_ui_tests)

set_tests_properties(ui_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
