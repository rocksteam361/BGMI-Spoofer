#!/system/bin/sh
MODDIR=${0%/*}

# Wait until Android is fully booted (prevents bootloops)
until [ "$(getprop sys.boot_completed)" = "1" ]; do
  sleep 1
done

# Spoof system properties
resetprop ro.serialno "SPOOFED_SERIAL_123"
resetprop ro.boot.wifimacaddr "02:00:00:00:00:01"
resetprop ro.boot.btmacaddr "02:00:00:00:00:02"
resetprop ro.product.model "Pixel 8 Pro"
resetprop ro.product.manufacturer "Google"
resetprop ro.build.fingerprint "google/husky/husky:14/UQ1A.240105.004/1234567:user/release-keys"

# Set proper permissions
chmod 755 $MODDIR/post-fs-data.sh
