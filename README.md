Processing H.264
================

This is a (prototype) interface for Processing applications to communicate with an h.264 encode/decode server through a native class. The server, written in C, uses x264 and libavcodec for the sake of compatibility with other operating systems such as Linux (or, in the future, Windows). The Processing classes interface with the C server processes with a standard local network socket. The protocol used between them is simplistic, using uncompressed RGB data as the decoded image format.

Proposed Use
===========

I imagine that this will be most useful for Processing developers who wish to transcode H.264 data without the need for x264 Processing bindings. Furthermore, I plan to use this myself for a webcam-oriented streaming application in the near future.
