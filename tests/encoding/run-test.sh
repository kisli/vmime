#/bin/sh

TEST_DIR="./test-suites"
TEMP_DIR="/tmp"
ENCODINGS="base64 quoted-printable uuencode"
PROGRAM="./main"


for encoding in $ENCODINGS ; do

	echo
	echo Testing encoding \'$encoding\'
	echo =====================================================================


	############
	#  Encode  #
	############

	echo ENCODE

	testFiles=`cd $TEST_DIR/encode ; find . -regex '\./[^\.]*' -maxdepth 1 -type f | tr -d ./`

	for testFile in $testFiles ; do

		if [ -e $TEST_DIR/encode/$testFile.$encoding ]
		then

			printf %20s "$testFile : "

			$PROGRAM $encoding e < $TEST_DIR/encode/$testFile > $TEMP_DIR/vmime_result

			diff="diff $TEMP_DIR/vmime_result $TEST_DIR/encode/$testFile.$encoding"
			res=`$diff`

			if [ "$res" = "" ]
			then
				echo "[OK]"
			else
				diffFile=$TEMP_DIR/vmime.encode.$encoding.$testFile.diff
				echo "[NO: diff file is $diffFile]"
				$diff > $diffFile
			fi

		fi

	done


	############
	#  Decode  #
	############

	echo DECODE [1/2]

	for testFile in $testFiles ; do

		if [ -e $TEST_DIR/encode/$testFile.$encoding ]
		then

			printf %20s "$testFile : "

			$PROGRAM $encoding d < $TEST_DIR/encode/$testFile.$encoding > $TEMP_DIR/vmime_result

			diff="diff $TEMP_DIR/vmime_result $TEST_DIR/encode/$testFile"
			res=`$diff`

			if [ "$res" = "" ]
			then
				echo "[OK]"
			else
				diffFile=$TEMP_DIR/vmime.decode.$encoding.$testFile.diff
				echo "[NO: diff file is $diffFile]"
				$diff > $diffFile
			fi

		fi

	done


	###########################################
	#  Decode from data not encoded by VMime  #
	###########################################

	echo DECODE [2/2]

	testFiles=`cd $TEST_DIR/decode ; find . -regex '\./[^\.]*' -maxdepth 1 -type f | tr -d ./`

	for testFile in $testFiles ; do

		if [ -e $TEST_DIR/decode/$testFile.$encoding ]
		then

			printf %20s "$testFile : "

			$PROGRAM $encoding d < $TEST_DIR/decode/$testFile.$encoding > $TEMP_DIR/vmime_result

			diff="diff $TEMP_DIR/vmime_result $TEST_DIR/decode/$testFile"
			res=`$diff`

			if [ "$res" = "" ]
			then
				echo "[OK]"
			else
				diffFile=$TEMP_DIR/vmime.decode2.$encoding.$testFile.diff
				echo "[NO: diff file is $diffFile]"
				$diff > $diffFile
			fi

		fi

	done

done

echo

