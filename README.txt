throttled 0.5.1 by quest, lws, and step76

This is a small app to provide bandwidth shaping to Mac OS X and FreeBSD. If you use Mac OS X and want a GUI version then check out http://intrarts.com/throttled.html

For more information visit http://intrarts.com/throttledcli.html or view the official throttled github repo at http://github.com/zquestz/throttled

usage: ./throttled [-Thv] -s speed -r rule [-i increment] [-d port] [-w weight]
-s speed        Max speed in bytes/second (required)
-r rule         IPFW rule number to remove when quit (required)
-i increment    Amount to change the throttle in bytes/sec (USR1 - Decrease | USR2 - Increase)
-T              Enable iTunes TTL fix
-h              This help screen
-v              Version information
-d port         Divert port (optional, may specify more than one)
    -w weight       Weight for the divert port specified prior to this option.

Whats New (0.5.1)
1. Added skipto rules for bonjour networks and 10.x.x.x networks. This should fix the remaining LAN issues people have reported.
2. Included sample configuration for Wired.

Whats New (0.5.0)
First I wanted to thank Stefano Ciccarelli for implementing weighted queues and becoming our first major contributor on the project. Without his work, this release would not have been possible. Now, on to the changes:
1. Implemented full weighted queues using WF2Q+.
2. Removed ACK priority and LAN uncapping features in the binary. They are now done in the throttled-startup file via ipfw rules.
3. Now compiles under Leopard.

Whats New (0.4.6) -
1. Updated priority_queue to use deque instead of vector for better memory management.

Whats New (0.4.5) -
1. Updated startup messages for local network throttling and updated the help/usage text.

Whats New (0.4.4) -
1. Compiled as a Universal Binary for PPC, PPC64, and Intel.

Whats New (0.4.3) -
1. Tweaked configuration files so they operate with more firewalls.
2. Changed some ipfw rule syntax to accommodate 10.3.9 users. 

Whats New (0.4.2) -
1. Modified the default config to not prioritize smtp or aim.
2. Included a few sample configs besides the default.
3. Added -v flag for version information.

Whats New (0.4.1) -
1. Fixed StartupItem for OS X.
2. Added divert rules for Ghost Recon.

Whats New (0.4) -
1. UDP support now works. This allows for VOIP and game priority options.
2. Simplified the rule removal code. See the -r flag. (Luigi--)
3. Compiles clean with gcc4 on Mac OS X 10.3-10.4.x, FreeBSD 4.x, and FreeBSD 5.x.
4. Linux support has been removed to clean up the codebase. 
5. The default throttled-startup script now has more rules, mainly for the VOIP and gaming features.

Whats New (0.3.2) -
1. Fixed FreeBSD pthread issues. Now should compile cleanly on 4.x and 5.x. 
2. Fixed a few packet priority bugs (thx Dave).

Whats New (0.3.1) -
1. Added iTunes TTL fix as -T flag.
2. Added default rule for Wired listing ports (port 2000)


Configuring and installing throttled (Binary Install for Mac OS X) 

1. cd to the directory containing throttled in the terminal.
2. edit throttled-startup with your desired configuration.
3. type "sudo ./Install.sh" to install throttled and its StartupItem.
4. to run throttled just type "sudo /usr/local/sbin/throttled-startup"


Configuring and installing throttled (Source Install) -

1. cd to the directory containing throttled in the terminal.
2. edit throttled-startup with your desired configuration.
3. type "make" ("gmake" on FreeBSD systems)
4. type "sudo make install"
5. to run throttled just type "sudo /usr/local/sbin/throttled-startup"
6. the source install does not include the startupitem, if you need that as well you can just run ./Install.sh


For more information you can read the comments in throttled-startup
Thats it... now throttled is all set to go.
