#!/bin/sh

TMPFILE=`mktemp`

touch $TMPFILE
rm $TMPFILE
h5dump-shared -A $1 >& $TMPFILE

for tr in $( grep TriggerRecord ${TMPFILE} | grep GROUP | sed -e 's/GROUP//g' | sed -e 's/ //g' | sed -e 's/"//g' | sed -e 's/{//g' )
do
    echo "Processing: " $tr
    for apa in {0..149}
    do
      apastr=`printf "APA%3.3d" $apa`
      cstring=""
      for link in {0..9}
      do
        linkstr=`printf "Link%2.2d" $link`
        dsdir=`echo /${tr}/TPC/${apastr}/${linkstr}`
        echo $dsdir
	trdu=`echo ${tr} | sed -e "s/\./_/g"`
	ofile=`echo ${trdu}TPC${apastr}${linkstr}.dat`
	echo $ofile
        h5dump-shared -d $dsdir -b LE -o $ofile $1
        cstring=`echo ${ofile} ${cstring}` 
      done
      apafile=`echo ${trdu}TPC${apastr}.dat`
      cat ${cstring} > ${apafile}
      rm ${cstring}
    done     
done

rm $TMPFILE

