## SimpleWeek (german)

Based on SimpleWeek (dutch):
https://github.com/atkaper/pebble-simpleweek-nl

Based on Simplicity & Complexity.

Binary release:
https://github.com/hhurz/pebble-simpleweek-de/raw/master/releases/simpleweek-de.pbw


### Building the watch face

Open a terminal window in the directory where you installed the SDK and change to the watches directory with this command:

    cd <sdk_path>/watches/
    
Recreate the symlinks to the SDK files with this command:
    
    ../tools/create_pebble_project.py --symlink-only ../sdk/ simpleweek-de
    
Configure the project using these commands:

    cd simpleweek-de

    ./waf configure build

Build the project using this command:

    ./waf build


### Install the watch face using dropbox (Android only)

    Copy simpleweek-de.pbw from <sdk_path>/watches/simpleweek-de/build to a dropbox folder

    Use your phone's dropbox app to download simpleweek-de.pbw to your phone


### Install the watch face using libpebble

First, make sure [libpebble](https://github.com/pebble/libpebble) is set up and running properly. Then:

    /<libpebble_path>/p.py --pebble_id <PEBBLE_ID_OR_MAC_ADDRESS> --lightblue load simpleweek-de.pbw


### Install the watch face using http server

Run:

    python -m SimpleHTTPServer 8000

Use your phone's web browser to browse to the pbw file in the build folder, and click it to install.


