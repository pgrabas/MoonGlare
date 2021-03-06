
group "Modules"

    project "Debug"
        kind "Makefile"
        MoonGlare.SetOutputDir("Modules")
        files "Debug/**"
        dependson "svfs"

        buildcommands {
            [["%{cfg.targetdir}/../svfs" -m "%{prj.basedir}/Debug" -e RDCExporter:%{cfg.targetdir}/Debug.rdc:/]],
        }

    project "Base"
        kind "Makefile"
        MoonGlare.SetOutputDir("Modules")
        files "Base/**"
        dependson "svfs"

        buildcommands {
            [["%{cfg.targetdir}/../svfs" -m "%{prj.basedir}/Base" -e RDCExporter:%{cfg.targetdir}/Base.rdc:/]],
        }
