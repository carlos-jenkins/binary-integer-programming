#!/bin/bash
for i in build-essential texlive libgtk-3-dev graphviz
do
    STATUS=$(dpkg-query -W --showformat='${Status}\n' $i | grep "install ok installed")
    echo "Testing $i : $STATUS"
    if [ "$STATUS" == "" ]; then
        sudo apt-get --yes install $i
    fi
done
