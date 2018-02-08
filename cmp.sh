EXE=./oldfraig
EXE=./fraig
REF=./fraig-linux

MYLOG=$1.mine.log
MYLOG2=$1.mine2.log
REFLOG=$1.ref.log
REFLOG2=$1.ref2.log

echo "Running my program..."
$EXE -f $1 1> $MYLOG 2> $MYLOG2
echo "Running ref program..."
$REF -f $1 1> $REFLOG 2> $REFLOG2

echo "diff..."
diff $MYLOG $REFLOG
diff $MYLOG2 $REFLOG2
#vimdiff $MYLOG $REFLOG

rm $MYLOG $REFLOG
rm $MYLOG2 $REFLOG2

