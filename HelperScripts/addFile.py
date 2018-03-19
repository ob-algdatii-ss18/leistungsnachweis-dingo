import datetime
import subprocess
from paths import path_to_src, path_to_cmake


def get_input_as_int():
    succeeded = False
    result = -1
    while not succeeded:
        try:
            result = int(input("Please choose: \n"))
            succeeded = True
        except Exception:
            pass
    return result


def output_file_comment(file, filename):
    file.write("/**\n"
               "*  @file    {}\n"
               "*  @author  \n"
               "*  @date    {}\n"
               "*/\n"
               "\n".format(filename, datetime.datetime.now().strftime("%d %B %Y")))


def output_header(name):
    filename = (name + ".h")

    with open(str(path_to_src / filename), "w") as file:
        output_file_comment(file, filename)
        file.write(
            "#pragma once\n"
            "\n"
            "namespace AGII\n"
            "{\n"
            "}  // namespace AGII\n"
        )


def output_source(name, has_header):
    filename = (name + ".cpp")
    with open(str(path_to_src / filename), "w") as file:
        output_file_comment(file, filename)
        if has_header:
            file.write('#include "{}"\n'.format((name + ".h")))
        file.write("\n"
                   "namespace AGII\n"
                   "{\n"
                   "}  // namespace AGII\n"
                   )


def output_shaders(name):
    pass


def main():
    try:
        while True:
            print("Header and Source Files")
            print("    1) Header File")
            print("    2) Source File")
            print("    3) Header and Source")
            print()

            print("Shaders")
            print("    4) Shaders (not implemented)")
            print()

            print("Miscellaneous")
            print("    0) Touch CMakeLists.txt")
            print("")

            choice = get_input_as_int()
            if choice != 0:
                filename = input("Please input the name (without extension):\n")
            if choice == 1 or choice == 3:
                output_header(filename)
            if choice == 2 or choice == 3:
                output_source(filename, choice == 3)
            if choice == 4:
                output_shaders(filename)

            # Touch CMake to make it regenerate
            path_to_cmake.touch()

            print("----------Success---------")
            print("")
            print("")
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()