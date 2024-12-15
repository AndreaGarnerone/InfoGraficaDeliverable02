cmake_minimum_required(VERSION 3.27)
project(OpenGLApp)

set(CMAKE_CXX_STANDARD 14)

include_directories(OpenGLApp)
include_directories(OpenGLApp/freetype)
include_directories("OpenGLApp/Examples/Example 10")
include_directories("OpenGLApp/Examples/Example 11")
include_directories("OpenGLApp/Examples/Example 12")
include_directories("OpenGLApp/Examples/Example 13")
include_directories("OpenGLApp/Examples/Example 14")
include_directories("OpenGLApp/Examples/Example 15")
include_directories("OpenGLApp/Examples/Example 16")
include_directories("OpenGLApp/Examples/Example 17")
include_directories("OpenGLApp/Examples/Example 18")
include_directories("OpenGLApp/Examples/Example 19")
include_directories("OpenGLApp/Examples/Example 20")
include_directories("OpenGLApp/Examples/Example 21")
include_directories("OpenGLApp/Examples/Example 22")
include_directories("OpenGLApp/Examples/Example 23")
include_directories("OpenGLApp/Examples/Example 8")
include_directories("OpenGLApp/Examples/Example 9")
include_directories(OpenGLApp/Exercises/Exercise3/Exercise3-1)
include_directories(OpenGLApp/Exercises/Exercise3/Exercise3-2)
include_directories(OpenGLApp/Exercises/Exercise3/Exercise3-3)
include_directories(OpenGLApp/Exercises/Exercise4/Exercise4-1)
include_directories(OpenGLApp/Exercises/Exercise4/Exercise4-2)
include_directories(OpenGLApp/Exercises/Exercise4/Exercise4-3)
include_directories(OpenGLApp/Exercises/Exercise5)
include_directories(OpenGLApp/Exercises/Exercise6)

add_executable(OpenGLApp
        "OpenGLApp/Examples/Example 10/Example_10.cpp"
        "OpenGLApp/Examples/Example 10/shader_s.h"
        "OpenGLApp/Examples/Example 10/stb_image.cpp"
        "OpenGLApp/Examples/Example 10/stb_image.h"
        "OpenGLApp/Examples/Example 11/Example_11.cpp"
        "OpenGLApp/Examples/Example 11/shader_s.h"
        "OpenGLApp/Examples/Example 11/stb_image.cpp"
        "OpenGLApp/Examples/Example 11/stb_image.h"
        "OpenGLApp/Examples/Example 12/Example_12.cpp"
        "OpenGLApp/Examples/Example 12/shader_s.h"
        "OpenGLApp/Examples/Example 12/stb_image.cpp"
        "OpenGLApp/Examples/Example 12/stb_image.h"
        "OpenGLApp/Examples/Example 13/Example_13.cpp"
        "OpenGLApp/Examples/Example 13/shader_s.h"
        "OpenGLApp/Examples/Example 13/stb_image.cpp"
        "OpenGLApp/Examples/Example 13/stb_image.h"
        "OpenGLApp/Examples/Example 14/Example_14.cpp"
        "OpenGLApp/Examples/Example 14/shader_s.h"
        "OpenGLApp/Examples/Example 14/stb_image.cpp"
        "OpenGLApp/Examples/Example 14/stb_image.h"
        "OpenGLApp/Examples/Example 15/Example_15.cpp"
        "OpenGLApp/Examples/Example 15/shader_s.h"
        "OpenGLApp/Examples/Example 15/stb_image.cpp"
        "OpenGLApp/Examples/Example 15/stb_image.h"
        "OpenGLApp/Examples/Example 16/Example_16.cpp"
        "OpenGLApp/Examples/Example 16/shader_s.h"
        "OpenGLApp/Examples/Example 16/stb_image.cpp"
        "OpenGLApp/Examples/Example 16/stb_image.h"
        "OpenGLApp/Examples/Example 17/Example17.cpp"
        "OpenGLApp/Examples/Example 17/shader_s.h"
        "OpenGLApp/Examples/Example 17/stb_image.cpp"
        "OpenGLApp/Examples/Example 17/stb_image.h"
        "OpenGLApp/Examples/Example 18/Example18.cpp"
        "OpenGLApp/Examples/Example 18/shader_s.h"
        "OpenGLApp/Examples/Example 18/stb_image.cpp"
        "OpenGLApp/Examples/Example 18/stb_image.h"
        "OpenGLApp/Examples/Example 19/camera.h"
        "OpenGLApp/Examples/Example 19/Example19.cpp"
        "OpenGLApp/Examples/Example 19/shader_s.h"
        "OpenGLApp/Examples/Example 19/stb_image.cpp"
        "OpenGLApp/Examples/Example 19/stb_image.h"
        "OpenGLApp/Examples/Example 20/camera.h"
        "OpenGLApp/Examples/Example 20/Example20.cpp"
        "OpenGLApp/Examples/Example 20/shader_s.h"
        "OpenGLApp/Examples/Example 20/stb_image.cpp"
        "OpenGLApp/Examples/Example 20/stb_image.h"
        "OpenGLApp/Examples/Example 21/camera.h"
        "OpenGLApp/Examples/Example 21/Example21.cpp"
        "OpenGLApp/Examples/Example 21/shader_s.h"
        "OpenGLApp/Examples/Example 21/stb_image.cpp"
        "OpenGLApp/Examples/Example 21/stb_image.h"
        "OpenGLApp/Examples/Example 22/camera.h"
        "OpenGLApp/Examples/Example 22/Example22.cpp"
        "OpenGLApp/Examples/Example 22/shader_s.h"
        "OpenGLApp/Examples/Example 22/stb_image.cpp"
        "OpenGLApp/Examples/Example 22/stb_image.h"
        "OpenGLApp/Examples/Example 23/camera.h"
        "OpenGLApp/Examples/Example 23/Main.cpp"
        "OpenGLApp/Examples/Example 23/shader_s.h"
        "OpenGLApp/Examples/Example 23/stb_image.cpp"
        "OpenGLApp/Examples/Example 23/stb_image.h"
        "OpenGLApp/Examples/Example 8/Example_08.cpp"
        "OpenGLApp/Examples/Example 8/shader_s.h"
        "OpenGLApp/Examples/Example 9/Example_09.cpp"
        "OpenGLApp/Examples/Example 9/shader_s.h"
        "OpenGLApp/Examples/Example 9/stb_image.cpp"
        "OpenGLApp/Examples/Example 9/stb_image.h"
        OpenGLApp/Examples/Example_00.cpp
        OpenGLApp/Examples/Example_01.cpp
        OpenGLApp/Examples/Example_02.cpp
        OpenGLApp/Examples/Example_03.cpp
        OpenGLApp/Examples/Example_04.cpp
        OpenGLApp/Examples/Example_05.cpp
        OpenGLApp/Examples/Example_06.cpp
        OpenGLApp/Examples/Example_07.cpp
        OpenGLApp/Exercises/Exercise2/Exercise_02-1.cpp
        OpenGLApp/Exercises/Exercise2/Exercise_02-2.cpp
        OpenGLApp/Exercises/Exercise2/Exercise_02-3.cpp
        OpenGLApp/Exercises/Exercise3/Exercise3-1/Exercise3-1.cpp
        OpenGLApp/Exercises/Exercise3/Exercise3-1/shader_s.h
        OpenGLApp/Exercises/Exercise3/Exercise3-2/Exercise3-2.cpp
        OpenGLApp/Exercises/Exercise3/Exercise3-2/shader_s.h
        OpenGLApp/Exercises/Exercise3/Exercise3-3/Main.cpp
        OpenGLApp/Exercises/Exercise3/Exercise3-3/shader_s.h
        OpenGLApp/Exercises/Exercise4/Exercise4-1/Exercise_4-1.cpp
        OpenGLApp/Exercises/Exercise4/Exercise4-1/shader_s.h
        OpenGLApp/Exercises/Exercise4/Exercise4-1/stb_image.cpp
        OpenGLApp/Exercises/Exercise4/Exercise4-1/stb_image.h
        OpenGLApp/Exercises/Exercise4/Exercise4-2/Exercise_4-2.cpp
        OpenGLApp/Exercises/Exercise4/Exercise4-2/shader_s.h
        OpenGLApp/Exercises/Exercise4/Exercise4-2/stb_image.cpp
        OpenGLApp/Exercises/Exercise4/Exercise4-2/stb_image.h
        OpenGLApp/Exercises/Exercise4/Exercise4-3/Exercise4-3.cpp
        OpenGLApp/Exercises/Exercise4/Exercise4-3/shader_s.h
        OpenGLApp/Exercises/Exercise4/Exercise4-3/stb_image.cpp
        OpenGLApp/Exercises/Exercise4/Exercise4-3/stb_image.h
        OpenGLApp/Exercises/Exercise5/Exercise5.cpp
        OpenGLApp/Exercises/Exercise5/shader_s.h
        OpenGLApp/Exercises/Exercise5/stb_image.cpp
        OpenGLApp/Exercises/Exercise5/stb_image.h
        OpenGLApp/Exercises/Exercise6/Exercise6.cpp
        OpenGLApp/Exercises/Exercise6/shader_s.h
        OpenGLApp/Exercises/Exercise6/stb_image.cpp
        OpenGLApp/Exercises/Exercise6/stb_image.h
        OpenGLApp/Exercises/Exercise_01.cpp
        OpenGLApp/camera.h
        OpenGLApp/ft2build.h
        OpenGLApp/glad.c
        OpenGLApp/Main.cpp
        OpenGLApp/shader_s.h
        OpenGLApp/stb_image.cpp
        OpenGLApp/stb_image.h)
