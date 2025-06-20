add_requires("entt", "gtest", "glm >=1.0.1", "glfw >=3.4", "glew", "spdlog", "fmt", "stb", "joltphysics", "miniaudio")

includes("../EngineSquared/xmake.lua")

set_project("VehicleDemo")
set_languages("c++20")

add_rules("mode.debug", "mode.release")

-- add /W4 for windows
if is_plat("windows") then
    add_cxflags("/W4")
end

target("VehicleDemo")
    set_kind("binary")
    set_default(true)
    add_deps("EngineSquared")

    add_files("src/**.cpp")
    add_includedirs("$(projectdir)/src/")
    add_includedirs("$(projectdir)/src/scene")

    add_packages("entt", "glm", "glfw", "glew", "spdlog", "fmt", "stb", "joltphysics", "miniaudio")

    set_rundir("$(projectdir)")


if is_mode("debug") then
    add_defines("ES_DEBUG")
    set_symbols("debug")
    set_optimize("none")
end

if is_mode("release") then
    set_optimize("fastest")
end
