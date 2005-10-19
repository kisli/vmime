#!/bin/sh

TEST_DIR="./test-suites"
TEMP_DIR="/tmp"
PROGRAM="./main"


testFiles=`cd $TEST_DIR ; find . -maxdepth 1 -regex '\./[^\.]*\.in\..*' -type f`

echo
echo Testing charset conversions
echo =====================================================================

for testFile in $testFiles ; do

	testName=`echo $testFile | sed 's/^\.\/\([^\.]*\).*/\1/'`
	sourceCharset=`echo $testFile | sed 's/^\.\/[^\.]*\.[^\.]*\.\(.*\)/\1/'`

	testOutFiles=`cd $TEST_DIR ; find . -maxdepth 1 -regex "\./$testName\.out\..*" -type f`

	for testOutFile in $testOutFiles ; do

		destCharset=`echo $testOutFile | sed 's/^\.\/[^\.]*\.[^\.]*\.\(.*\)/\1/'`

		printf %20s "$testName "
		printf %30s "$sourceCharset --> $destCharset : "

		$PROGRAM $sourceCharset $destCharset < $TEST_DIR/$testFile > $TEMP_DIR/vmime_result

		diff="diff $TEMP_DIR/vmime_result $TEST_DIR/$testOutFile"
		res=`$diff`

		if [ "$res" = "" ]
		then
			echo "[OK]"
		else
			diffFile=$TEMP_DIR/vmime.charset.$testName.$sourceCharset.$destCharset.diff
			echo "[NO: diff file is $diffFile]"
			$diff > $diffFile
		fi

	done

done

echo

