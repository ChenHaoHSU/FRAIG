/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include "myHashMap.h"

using namespace std;

// TODO: define your own typedef or enum

/**********************************************************/
/*   Type Declaration                                     */
/**********************************************************/
class CirGate;
class CirMgr;
class SatSolver;


/**********************************************************/
/*   Macros                                               */
/**********************************************************/
#define VAR(x)     (x / 2)
#define INV(x)     (x % 2)
#define LIT(x, y)  ((x * 2) + y)

#define CONST1     (0x1)
#define ALL1       (~(0x1))

#define BYTE_SIZE  (8)
#define SIZET_SIZE (sizeof(size_t) * BYTE_SIZE)

/**********************************************************/
/*   Global Varibles                                      */
/**********************************************************/

/**********************************************************/
/*   Global enum                                          */
/**********************************************************/
enum OptType   // for optimization (4 cases)
{
	OPT_CONST0    = 0,
	OPT_CONST1    = 1,
	OPT_SAMEFANIN = 2,
	OPT_INVFANIN  = 3,
	OPT_NONE      = 4
};



#endif // CIR_DEF_H
