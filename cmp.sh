EXE=./fraig
REF=./fraig-64

MYLOG=$1.mine.log
REFLOG=$1.ref.log

$EXE -f $1 1> $MYLOG
$REF -f $1 1> $REFLOG 

diff $MYLOG $REFLOG

#rm $MYLOG $REFLOG
