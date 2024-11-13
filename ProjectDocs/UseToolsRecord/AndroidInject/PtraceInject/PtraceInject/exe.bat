echo %cd%
adb shell su -c "rm /data/local/tmp/source13/inject"
adb push "libs\armeabi-v7a\inject" "/data/local/tmp/source13/inject"
adb shell su -c "chmod 777 /data/local/tmp/source13/*"
adb shell su -c "/data/local/tmp/source13/inject"
