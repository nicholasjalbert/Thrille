import marshal
import sys      


if len(sys.argv) != 3:
    print "Usage: python removeme.py [input dictionary]",
    print "[output dictionary]"
    print 
    print "Iterates through a dictionary and allows you to",
    print "remove specific entries"
    print
    sys.exit(0)

print "Loading", sys.argv[1]

p = marshal.load(open(sys.argv[1], "r"))

print "Type \"y\" to remove entry, all other input will be",
print "ignored and allow aentry to remain"
for x in p.keys():
    delme = raw_input("delete " + x + "?")
    if "y" in delme:
        del p[x]


print "Result"
print p


p = marshal.dump(p, open(sys.argv[2], "w"))
