./fern -D miniseed   -e 'usgs:usp0006dzc' -n XE -d +30m -c BHZ -p t/test_miniseed -o t/test_miniseed.request.test

# Check Request
diff t/test_miniseed.request.test t/test_miniseed.request || exit -1



function command_find() {
    for cmd in $*; do
        OUT=$(command -v $cmd)
        RETVAL=$?
        if [ $RETVAL == 0 ]; then
            echo ${OUT}
            exit 0
        fi
    done
    exit -1
}

MD5=$(command_find md5sum md5) || { echo >&2 "Error: could not find md5.  Aborting."; exit 1; }
echo "MD5: $MD5"


# Check File
md5_file=$(${MD5} -q t/test_miniseed.*.IRISDMC.mseed)

# Generated MD5 Sum on original file
md5_check="f65705013ecd6b0e944a83e0c54fcbf2"
if [[ $md5_check != $md5_file ]]; then
    echo "MD5 FILE:     $md5_file"
    echo "MD5 EXPECTED: $md5_check"
    ls -l t/test_miniseed.*.mseed
    exit -1
fi
rm -f t/test_miniseed*mseed
