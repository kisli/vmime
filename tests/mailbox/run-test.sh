#!/bin/sh

TEST_DIR="./test-suites"
TEMP_DIR="/tmp"
PROGRAM="./main"


testFiles=`cd $TEST_DIR ; ls *.in`

echo
echo Testing address parsing
echo =====================================================================

for testFile in $testFiles ; do

	testName=`echo $testFile | sed 's/\([^\.]*\)\.in/\1/'`

	printf %20s "$testName : "

	$PROGRAM < $TEST_DIR/$testFile > $TEMP_DIR/vmime_result

	diff="diff $TEMP_DIR/vmime_result $TEST_DIR/$testName.out"
	res=`$diff`

	if [ "$res" = "" ]
	then
		echo "[OK]"
	else
		diffFile=$TEMP_DIR/vmime.mailbox.$testName.diff
		echo "[NO: diff file is $diffFile]"
		$diff > $diffFile
	fi

done

echo

