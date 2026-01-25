-- Build with xmake -P .
set_project("ES-RS")
set_languages("c++20")

if is_plat("windows") then
    add_cxflags("/W4")
end

includes("../EngineSquared/xmake.lua")

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
target("ES-RS")
    set_kind("binary")

    add_deps("EngineSquared")

    add_files("src/**.cpp")

    add_includedirs("$(projectdir)/src/")

    add_packages("entt", "glm", "glfw", "spdlog", "fmt", "joltphysics", "stb", "tinyobjloader", "wgpu-native", "glfw3webgpu", "lodepng", "rmlui", "miniaudio")

    set_rundir("$(projectdir)")

if is_mode("debug") then
    add_defines("ES_DEBUG")
    set_symbols("debug")
    set_optimize("none")
end

if is_mode("release") then
    set_optimize("fastest")
end
