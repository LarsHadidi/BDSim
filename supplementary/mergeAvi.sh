list=`echo ${1}-*.avi | sed 's/ /\\|/g'`
avconv -y -i concat:$list -c copy ${1}.avi
