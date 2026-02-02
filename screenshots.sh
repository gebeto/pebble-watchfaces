#! /bin/bash

function make_screenshot() {
    dir=$1;
    echo "Processing $dir"
    cd $dir

    mkdir -p assets

    echo "#define DARK_MODE 0" > src/config.h
    pebble kill
    make run
    sleep 2
    pebble screenshot assets/screenshot.png --no-open

    echo "#define DARK_MODE 1" > src/config.h
    # pebble kill
    make run
    sleep 2
    pebble screenshot assets/screenshot~dark.png --no-open

    git checkout -- src/config.h
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

