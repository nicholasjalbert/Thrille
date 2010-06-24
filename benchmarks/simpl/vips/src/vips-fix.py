# Some clashes in declarations caused vips not to compile.
# This script automatically resolves those clashes
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#



import os
import sys



def fixColoursHeader():
    print "Fixing colours.h header..."
    cwd = os.getcwd()
    libvips_path = os.path.join(cwd, "vips-7.20.0", "libvips")
    assert os.path.exists(libvips_path)
    colour_header = os.path.join(libvips_path, "include", "vips", "colour.h")
    assert os.path.exists(colour_header)
    colour = open(colour_header, "r").readlines()
    
    for i in range(0, len(colour)):
        if "intent" in colour[i]:
            colour[i] = colour[i].replace("VipsIntent", "int")

    colour_out = open(colour_header, "w")
    for x in colour:
        colour_out.write(x)
    colour_out.close()

def fixDeprecated():
    print "Fixing rename.c file..."
    cwd = os.getcwd()
    libvips_path = os.path.join(cwd, "vips-7.20.0", "libvips")
    assert os.path.exists(libvips_path)
    rename_path = os.path.join(libvips_path, "deprecated", "rename.c")
    assert os.path.exists(rename_path)
    rename = open(rename_path, "r").readlines()

    for i in range(0, len(rename)):
        if "im_icc_export( IM" in rename[i]:
            if "/*" in rename[i - 2]:
                pass
            else:
                rename.insert(i-1, "/*\n")

        if "8, output" in rename[i]:
            if "*/" in rename[i + 2]:
                pass
            else:
                rename.insert(i+2, "*/\n")

    rename_out = open(rename_path, "w")
    for x in rename:
        rename_out.write(x)
    rename_out.close()



def main():
    print "Fix problematic VIPS function declarations..."
    fixColoursHeader()
    fixDeprecated()


if __name__ == "__main__":
    main()
