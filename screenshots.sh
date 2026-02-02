#! /bin/bash

function make_screenshot() {
    dir=$1;
    echo "Processing $dir"
    cd $dir
    pebble kill
    make run
    sleep 3
    mkdir -p assets
    pebble screenshot assets/screenshot.png --no-open
    cd ..
}

if [ "$1" == 'all' ]; then
    echo "Do All"
    for dir in */ ; do
        make_screenshot dir
    done
elif [ -d "$1" ]; then
    make_screenshot $1
else
    echo "No command. Exiting..."
fi

