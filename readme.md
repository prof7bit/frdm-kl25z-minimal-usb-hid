# Minimal USB HID example for Kinetis L

This implements a minimalistic USB protocol stack for the USB peripheral of a Kinetis L microcontroller (specifically a KL25 on the FRDM demo board) without all the bloat introduced by their SDK and their Kinetis Design Studio. Flash size for a bare bones USB example is reduced from 35 kB (official KDS & SDK) to just 2.5 kB (this project).

This is a work in progress, its probably not yet finished and far from production ready, but it might already serve as a help for people looking for example code and trying to implement USB on the smaller and cheaper Kinetis devices which does not seem feasible with KDS and their bloated SDK at the moment.