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

# Open file
fout = open(sys.argv[1], 'w')

# First line
fout.write('aag {} {} {} {} {}\n'.format(maxIdx, nPI, nLATCH, nPO, nAIG))

#PIs
for i in range(nPI):
	fout.write('{}\n'.format(piIdx[i] * 2))

#POs
for i in range(nPO):
	fout.write('{}\n'.format(poIdx[i] * 2 + rand.randint(0, 1)))

#AIGs
preGates = piIdx
for i in range(nAIG):
	v = aigIdx[i] * 2
	f1 = rand.choice(preGates) * 2 + rand.randint(0, 1)
	f2 = rand.choice(preGates) * 2 + rand.randint(0, 1)
	fout.write('{} {} {}\n'.format(v, f1, f2))
	preGates.append(aigIdx[i])
	
#Symbols
for i in range(nPI):
	fout.write('i{} INPUT_{}\n'.format(i,i))
for i in range(nPO):
	fout.write('o{} OUTPUT_{}\n'.format(i,i))

#Comments
fout.write('c\nAAG output by Chen-Hao Hsu\n')