radeon-pm-gui
=============

Radeon Power Management GUI

This program aims to provide a GTK front-end to the Radeon sysfs power
management interfaces on the Linux kernel.

I've got a Radeon HD 6850, 3200 IGP, and a Llano CPU as test platforms, so
those are my primary targets. According to the RadeonFeature page on X.org [1],
this should work for r100 and up.

I'm hoping to support multiple graphics cards in a single system with the 1.0
version, and eventually this might grow to support reclocking of Intel/Nvidia
chips.

The initial target is the sysfs interfaces for core/memory clock speeds and 
performance profiles.  Later, fan management and display power management
options might also be added when applicable.

Also in the near term, temperature reporting will also be added for cards
which expose thermal sensors via the drm sysfs interface.

GTK3 is hopefully going to be the only hard dependency.

As far as license goes, this software is released under the GPL version 2. See
LICENSE in the root of the source distibution for the full license text.

[1] - http://www.x.org/wiki/RadeonFeature

