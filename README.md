CylonPIR
========

Author: Brian Gershon, Dec 2013

## Overview

An Arduino sketch that moves a Cylon eye along a 32 LEDs strip. Things change when the PIR sensor is triggered.

When first powered up, all LEDs are red and count down for 30 seconds to give PIR a chance to register the room without any movement.

Then will all LEDs off, a dim Cyclon eye scrolls back and forth.

If you move in front of the PIR you trigger a fast scrolling mode which sets all LEDs to blue, with a bright red Cyclon eye moving back and forth.

## Details

I used trigonometry to figure out how long to delay the eye so that it's fast in the center and appears to wrap around and slow down toward the ends, as if the LEDs were curved rather than in a straight line.

Since there are only 32 LEDs, the ASIN trig function turned out to be best since I could find the delay based on specific angles. Calcs are in spreadsheet. 
