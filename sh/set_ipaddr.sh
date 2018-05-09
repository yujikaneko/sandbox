#!/bin/sh

setip_dhcp() {
    echo "DHCP" $1
}

setip_static() {
    echo "static" $1 $2
}

setip() {
    if [ "$2" = "dhcp" ]; then
        setip_dhcp $1
    else
        IP=""
        CHKIP="0"
        OCTS=`echo $2 | awk -F. '{print $1 " " $2 " " $3 " " $4}'`
        for OCT in ${OCTS}
        do
            if [ "${OCT}" = "" ]; then
                break
            fi
            RNG=`expr 0 \<= ${OCT} \&  ${OCT} \<= 255`
            if [ $? -ge 2 ]; then
                break
            fi
            if [ ${RNG} -ne 1 ]; then
                break
            fi
            CHKIP=`expr ${CHKIP} + 1`
            if [ "${CHKIP}" -lt 4 ]; then
                IP=${IP}${OCT}"."
            else
                IP=${IP}${OCT}
                break
            fi
        done
        if [ "${CHKIP}" -eq 4 ]; then
           setip_static $1 ${IP}
        else
           echo "IP ADDRESS ERROR!"
           exit 1
        fi
    fi
}

# main

#NLIST=`ip a | grep ^[0-9] | sed -r 's/.*[: ](.*)[:].*/\1/'`
NLIST=`ip a | awk -F": " '$1~/^[0-9]/{print $2}'`

for NAME in ${NLIST}
do
   echo ${NAME} | grep ^en > /dev/null
   if [ $? -eq 0 ]; then
       setip ${NAME} $1
   fi
done

