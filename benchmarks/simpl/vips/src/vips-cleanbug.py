# This script removes the seeded bug from VIPS before building it
#
# Author: Nick Jalbert (jalbert@eecs.berkeley.edu)
#

import os
import sys

# This function uncomments out the lock and unlock in im_semaphore_upn 
# (file vips-7.20.0/libvips/iofuncs/semaphore.c)
# at lines 87 and 92 respectively

def cleanBugInSemaphore():
    print "Removing bug from semaphore.c..."
    cwd = os.getcwd()
    libvips_path = os.path.join(cwd, "vips-7.20.0", "libvips")
    assert os.path.exists(libvips_path)
    sem_path = os.path.join(libvips_path, "iofuncs", "semaphore.c")
    assert os.path.exists(sem_path)
    sem_c = open(sem_path, "r").readlines()
    assert "g_mutex_lock( s->mutex )" in sem_c[86]
    assert "g_mutex_unlock( s->mutex )" in sem_c[91]

    sem_c[86] = "        g_mutex_lock( s->mutex );\n"
    sem_c[91] = "        g_mutex_unlock( s->mutex );\n"

    sem_out = open(sem_path, "w")
    for x in sem_c:
        sem_out.write(x)
    sem_out.close()


def placeGassertsInThreadgroup():
    print "Changing thrilleAssertC to g_assert in threadgroup.c..."
    cwd = os.getcwd()
    libvips_path = os.path.join(cwd, "vips-7.20.0", "libvips")
    assert os.path.exists(libvips_path)
    threadgroup_path = os.path.join(libvips_path, "iofuncs", "threadgroup.c")
    assert os.path.exists(threadgroup_path)
    threadgroup_c = open(threadgroup_path, "r").readlines()
    for i in range(0, len(threadgroup_c)):
        tmp = threadgroup_c[i].replace("thrilleAssertC", "g_assert")
        threadgroup_c[i] = tmp

    threadgroup_out = open(threadgroup_path, "w")
    for x in threadgroup_c:
        threadgroup_out.write(x)
    threadgroup_out.close()

def main():
    cleanBugInSemaphore()
    placeGassertsInThreadgroup()


if __name__ == "__main__":
    main()
