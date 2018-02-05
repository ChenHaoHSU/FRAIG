#############################################################################
#  FileName     [ patternGen.py ]
#  PackageName  []
#  Synopsis     [ Pattern file random generator ]
#  Author       [ Chen-Hao Hsu ]
#  Date         [ 2018/2/6 created ]
#############################################################################

import sys
import random as rand

if len(sys.argv) != 4:
   print("Usage: python3 patternGen.py <fileName> <#PI> <#Pattern>")
   exit()

nPI = int(sys.argv[2])
nPtn = int(sys.argv[3])

fout = open(sys.argv[1], 'w')
for i in range(nPtn):
   for j in range(nPI):
      if (rand.random() < 0.5):
         fout.write('0')
      else:
         fout.write('1')
   fout.write('\n')

print("==============================")
print("Output \"" + sys.argv[1] + "\"")
print("#Pattern = " + sys.argv[2])
print("#PI      = " + sys.argv[3])
print("==============================")
