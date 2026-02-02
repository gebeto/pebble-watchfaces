cd a158
pebble kill
make run
sleep 3
pebble screenshot assets/screenshot.png --no-open
mkdir -p assets
mv pebble_screenshot_* assets/screenshot.png