#!/usr/bin/env python3

import json
import itertools
import argparse
import pathlib

# since writing cmake presets by hand is quite repetitive (ie i have to specify build type for each platform), i decided to write this program to generate all of the various presets. the way i prefer writing presets follows a simple template, so it is easy to just generate all the platform/build type/toolchain/compiler options combinations

"""
old comments from the old presets file explaining some choices made:

"$comment": [
        "using $comment requires at least version 10.",
        "using workflowPresets requires at least version 6.",
        "",
        "NOTE: the stm32f3-* presets compile with the gcc cross compiler with apprpriate flags for the f3 mcu. this tests that the library will successfully cross compile.",
        "NOTE: the apple-clang-* presets compile with the clang compiler on a host computer with a bunch of useful flags to facilitate catching mistakes. this enables us to write tests targeting the host machine so we can test behavior more easily",
        "USAGE: 'cmake --workflow apple-clang-debug' [build and run the tests on the host]"
    ],

"$comment": [
                "NOTE: we must pass CMAKE_PROGRAM_PATH to tell the toolchain file where to look for the arm-none-eabi toolchain. it must be an ENV variable since cacheVariables are not loaded until after the toolchain file is processed",
                "TODO: delete this. this path is only useful on my machine since i have a unique installation locatino for the cross-compiler. other users should specify their own arm-none-eabi location, or just install it in the normal places cmake will look for it"
            ],
            "environment": {
                "CMAKE_PROGRAM_PATH": "/Users/colinhk/code/stm32/arm-gnu-toolchain-15.2.rel1-darwin-arm64-arm-none-eabi/bin/"
            }


"$comment": [
                "NOTE: the stm32f3 toolchain workflow simply attempts to compile the library using the appropriate cross compiler. comprehensive tests are run on the host, and so should use a host machine workflow"
            ],
"""


def main(args: argparse.Namespace):
    base_config = {
        "name": "base-config",
        "hidden": True,
        "generator": "Ninja",
        "binaryDir": "${sourceDir}/build/${presetName}",
        "cacheVariables": {
            "CMAKE_EXPORT_COMPILE_COMMANDS": "ON",
            "CMAKE_CXX_STANDARD": "23",
            "CMAKE_CXX_STANDARD_REQUIRED": "ON",
            "CMAKE_CXX_EXTENSIONS": "OFF",
            "CMAKE_COLOR_DIAGNOSTICS": "ON",
        },
    }
    base_build = {"name": "base-build", "hidden": True, "jobs": 4}
    base_test = {
        "name": "base-test",
        "hidden": True,
        "output": {"outputOnFailure": True},
        "execution": {"noTestsAction": "error", "stopOnFailure": True},
    }

    # specify the various configuration combinations
    build_types = ("debug", "release")
    platform_names = ("stm32f3", "apple-clang", "gcc")
    toolchain_filenames = ("arm-none-eabi-gcc.cmake", "apple-clang.cmake", "gcc.cmake")

    configure_presets = [base_config]
    build_presets = [base_build]
    test_presets = [base_test]
    workflow_presets = []

    # TODO: notice there is some careful logic to only add certain specifications to certain platforms. find a better/more elegant way to do this. perhaps it should be an additional parameter specified in the configuration combinations above ?
    for build_type, (platform_name, toolchain_filename) in itertools.product(
        build_types, zip(platform_names, toolchain_filenames)
    ):
        name_str = f"{platform_name}-{build_type}"

        configure_preset = {
            "name": name_str,
            "inherits": "base-config",
            "toolchainFile": f"${{sourceDir}}/cmake/toolchains/{toolchain_filename}",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": f"{build_type}",
            },
        }
        if platform_name != "stm32f3":
            configure_preset["cacheVariables"]["PROJECT_ENABLE_TESTING"] = True
        configure_presets.append(configure_preset)

        build_preset = {
            "name": name_str,
            "inherits": "base-build",
            "configurePreset": name_str,
        }
        build_presets.append(build_preset)

        # we do not run tests on the stm32f3 platform
        if platform_name != "stm32f3":
            test_preset = {
                "name": name_str,
                "inherits": "base-test",
                "configurePreset": name_str,
            }
            test_presets.append(test_preset)

        workflow_preset = {
            "name": name_str,
            "steps": [
                {"type": "configure", "name": name_str},
                {"type": "build", "name": name_str},
            ],
        }
        if platform_name != "stm32f3":
            workflow_preset["steps"].append(
                {"type": "test", "name": name_str},
            )
        workflow_presets.append(workflow_preset)

    # assemble the final preset json object
    presets_object = {
        "version": 10,
        "configurePresets": configure_presets,
        "buildPresets": build_presets,
        "testPresets": test_presets,
        "workflowPresets": workflow_presets,
    }

    presets_json = json.dumps(presets_object, indent=4)

    if args.commit:
        with args.filepath.open("w") as file:
            file.write(presets_json)

        print(
            f"finished generating cmake presets.\nwriting to {args.filepath.resolve()}"
        )
    else:
        print(f"generated presets string: (not saved to file)\n\n{presets_json}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="generate a CMake preset file with the given name"
    )
    parser.add_argument(
        "filepath",
        type=pathlib.Path,
        help="name of file to which the generated preset json object is written. i think this path is relative to the directory from which this executable is invoked",
    )
    parser.add_argument(
        "-c",
        "--commit",
        action="store_true",
        help="if passed, actually perform the write to the file, overwriting any existing data in that file. otherwise, print the generated preset json object to the console",
    )
    main(parser.parse_args())
