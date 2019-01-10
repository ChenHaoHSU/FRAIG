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
      fout.write('{}'.format(rand.randint(0, 1)))
   fout.write('\n')

print('{} {}-bit patterns generated. ({})'.format(nPtn, nPI, sys.argv[1]))
