#!/usr/bin/env python
import os
import sys
import subprocess

from methods import print_error


libname = "opus-node"
projectdir = "project"

localEnv = Environment(tools=["default"], PLATFORM="")

# Build profiles can be used to decrease compile times.
# You can either specify "disabled_classes", OR
# explicitly specify "enabled_classes" which disables all other classes.
# Modify the example file as needed and uncomment the line below or
# manually specify the build_profile parameter when running SCons.

# localEnv["build_profile"] = "build_profile.json"

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

if not (os.path.isdir("godot-cpp") and os.listdir("godot-cpp")):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

env = SConscript("godot-cpp/SConstruct", {"env": env, "customs": customs})

def build_opus(platform):
    opus_dir = "thirdparty/opus-1.6.1"
    build_dir = os.path.join(opus_dir, "build")

    if platform == "windows":
        expected_lib = os.path.join(build_dir, "Release", "opus.lib")
    else:
        expected_lib = os.path.join(build_dir, "libopus.a")
        
    if os.path.exists(expected_lib):
        print("Opus static library found. Skipping CMake step.")
        return

    print(f"Building Opus for {platform} via CMake...")
    os.makedirs(build_dir, exist_ok=True)

    configure_cmd = [
        "cmake", "..", 
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", 
        "-DCMAKE_BUILD_TYPE=Release"
    ]
    subprocess.run(configure_cmd, cwd=build_dir, check=True)

    build_cmd = [
        "cmake", "--build", ".", 
        "--config", "Release"
    ]
    subprocess.run(build_cmd, cwd=build_dir, check=True)

build_opus(env["platform"])

env.Append(CPPPATH=["thirdparty/opus-1.6.1/include"])

if env["platform"] == "windows":
    env.Append(LIBPATH=["thirdparty/opus-1.6.1/build/Release"])
    env.Append(LIBS=["opus"])
    env.Append(LINKFLAGS=[
        "/NODEFAULTLIB:MSVCRT", 
        "/IGNORE:4217", 
        "/IGNORE:4286"
    ])
else:
    env.Append(LIBPATH=["thirdparty/opus-1.6.1/build"])
    env.Append(LIBS=["opus"])

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# .dev doesn't inhibit compatibility, so we don't need to key it.
# .universal just means "compatible with all relevant arches" so we don't need to key it.
suffix = env['suffix'].replace(".dev", "").replace(".universal", "")

lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "bin/{}/{}".format(env['platform'], lib_filename),
    source=sources,
)

copy = env.Install("{}/bin/{}/".format(projectdir, env["platform"]), library)

default_args = [library, copy]
Default(*default_args)
