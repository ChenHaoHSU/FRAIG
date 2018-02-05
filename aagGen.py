#############################################################################
#  FileName     [ aagGen.py ]
#  PackageName  []
#  Synopsis     [ AAG file random generator ]
#  Author       [ Chen-Hao Hsu ]
#  Date         [ 2018/2/6 created ]
#############################################################################

import sys
import random as rand

if len(sys.argv) != 7:
   print("Usage: python3 aagGen.py <fileName> <#MaxIdx> <#PI> <#LATCH> <#PO> <#AIG>")
   print("Example: python3 aagGen.py 1.aag 1000 100 0 100 200")
   exit()

maxIdx = int(sys.argv[2])
nPI    = int(sys.argv[3])
nLATCH = int(sys.argv[4])
nPO    = int(sys.argv[5])
nAIG   = int(sys.argv[6])

# Error handle (a little)
assert maxIdx >= nPI + nAIG
assert nLATCH == 0

allIdx = [ (i+1) for i in range(maxIdx) ]
rand.shuffle(allIdx)
piIdx  = [ allIdx.pop() for i in range(nPI)  ]
aigIdx = [ allIdx.pop() for i in range(nAIG) ]
rand.shuffle(aigIdx)
poIdx = [ rand.choice(aigIdx) for i in range(nPO) ]

fout = open(sys.argv[1], 'w')
# First line
fout.write('aag '+str(maxIdx)+' '+str(nPI)+' '+str(nLATCH)+' '+str(nPO)+' ' +str(nAIG)+'\n')
#PIs
for i in range(nPI):
	fout.write(str(piIdx[i] * 2) + '\n')
#POs
for i in range(nPO):
	fout.write(str(poIdx[i] * 2 + rand.randint(0, 1)) + '\n')
#AIGs
preGates = piIdx
for i in range(nAIG):
	fout.write(str(aigIdx[i] * 2))
	fout.write(' ' + str(rand.choice(preGates) * 2 + rand.randint(0, 1)))
	fout.write(' ' + str(rand.choice(preGates) * 2 + rand.randint(0, 1)))
	fout.write('\n')
	preGates.append(aigIdx[i])
#Symbols
for i in range(nPI):
	fout.write('i'+str(i)+' '+'INPUT'+str(i)+'\n')
for i in range(nPO):
	fout.write('o'+str(i)+' '+'OUTPUT'+str(i)+'\n')
#Comments
fout.write('c\nAAG output by Chen-Hao Hsu\n')