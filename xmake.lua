-- Build with xmake -P .
set_project("ES-RS")
set_languages("c++20")
add_rules("mode.debug", "mode.release")


if is_plat("windows") then
    add_cxflags("/W4")
end

add_repositories("EngineSquaredrepo https://github.com/EngineSquared/xrepo.git")

add_requires("enginesquared ab1b71caa1110642a5e4ee3a56205ce428fb8cd0", { debug = is_mode("debug") })

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})
target("ES-RS")
    set_kind("binary")

    add_packages("enginesquared")

    add_files("src/**.cpp")

    add_includedirs("$(projectdir)/src/")

    set_rundir("$(projectdir)")

    if is_mode("debug") then
        add_defines("ES_DEBUG")
        set_symbols("debug")
        set_optimize("none")
    end

    if is_mode("release") then
        set_optimize("fastest")
    end
target_end()
