# This is a very simple installation script for throttled.

# Install binaries and throttled-startup script.
echo "Installing throttled 0.5.1 binary."
install -d /usr/local/sbin
install -c throttled /usr/local/sbin/throttled
install -c throttled-startup /usr/local/sbin/throttled-startup

# Install Mac OS X StartupItem.
echo "Installing throttled StartupItem."
install -d /Library/StartupItems/throttledStartup
install -c throttledStartup/StartupParameters.plist /Library/StartupItems/throttledStartup/StartupParameters.plist
install -c throttledStartup/throttledStartup /Library/StartupItems/throttledStartup/throttledStartup

echo "Installation complete."