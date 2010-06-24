# Cleans up a problem in the gettext distribution
# 
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#



import os
import sys




def fixWriteCatalog():
    print "Fixing write-catalog.c file..."
    cwd = os.getcwd()
    gettext_path = os.path.join(cwd, "gettext-0.17", "gettext-tools", "src")
    assert os.path.exists(gettext_path)
    write_catalog = os.path.join(gettext_path, "write-catalog.c")
    assert os.path.exists(write_catalog)
    file = open(write_catalog, "r").readlines()
    
    for i in range(0, len(file)):
        if "fd = open" in file[i]:
            file[i] = "          fd = open "
            file[i] += "(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);\n"

    catalog_out = open(write_catalog, "w")
    for x in file:
        catalog_out.write(x)
    catalog_out.close()



def main():
    print "Fix problematic gettext function call..."
    fixWriteCatalog()


if __name__ == "__main__":
    main()
