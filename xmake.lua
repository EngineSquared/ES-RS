add_repositories("package_repo https://github.com/EngineSquared/xrepo.git")

add_requires("enginesquared v0.1.0")

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
    add_packages("enginesquared")

    add_files("src/**.cpp")
    add_includedirs("$(projectdir)/src/")
    add_includedirs("$(projectdir)/src/scene")

    set_rundir("$(projectdir)")


if is_mode("debug") then
    add_defines("ES_DEBUG")
    set_symbols("debug")
    set_optimize("none")
end

if is_mode("release") then
    set_optimize("fastest")
end
