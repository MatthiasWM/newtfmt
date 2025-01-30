# Newton Package Format

The Newton Package format serves as a container format for Newton applications,
books, fonts, and other extensions. A Package starts with a directory, followed
by one or more Parts. The newer format 1 also supports a relocation table.

The basics of the Package format are described in "Newton Formats Version 1.1".

## Package Directory

Packages start with the ASCII character sequence "package0" or "package1". 
Format 0 is used for Newton OS 1.x devices. OS 2.x can read format 0 and 1.
The formats differ in alignment. Format 1 has some additional features.

The directory contains a header, one directory entry per part, followed by a 
data section, holding strings for all directory entries. 

There is a "hidden" space after the last directory data, but before the start 
of the first part. Here we find an optionl message from the app that generated 
the package, for example:

 - "Newton™ ToolKit Package © 1992-1993, Apple Computer, Inc." with the later 
    year set at 1993, 1994, 1995, 1997
 - "NewtPack 3.3-2 © 1995-96, S. Weyer (portions may be derived from Newton™ 
    Toolkit © 1992-95, Apple Computer, Inc.)" with the later year set at '96, 
    '98
 - "NewtPack 3.5d-2 1995-2001, S. Weyer" and other versions
 - "Newt 3.0 © 1995, S. Weyer..."
 - "Newton Toolkit 1.6.4b3" with versions 1.6.4b2, 1.6.4b2c2, 1.6.4b3, 1.6.4
 - "Built with Newton Press"
 - "@1997 deep focus designs. All rights reserved. Sound Icon Maker.", "Icon Maker."
 - "lKit Package ? 1992-1995, Apple Computer, Inc."

It's nice to see that there were at least two other ways to generate packages
besides NTK, which was commercial and cost around $1000US back then. But it also
means that our package files differ in little details.

## Package Relocation 

An optional block in format 1 can hold relocation data. While Newton Object
Parts can relocate themselves, ARM assembler code that could be included inside
a NOS Part may need relocation data. I have yet to find a Package that uses
relocation data.

## Package Parts

Package Parts can have four possible types, "Protocol", "NOS", "Raw", or
"Package". The documentation describes only NOS parts. We still have to find 
out about more the other parts. From what I gathered, Packages within Packages
are the NewtonOS version of C++ object files that can be used for linking.

## NOS Parts

NOS Parts have several subtypes, "form" and "book" being the most common. The
contents itself is a list of NewtonScript objects, arranged in the same way they
will be in RAM or ROM for an ARM32 CPU. 

In a normalized NOS Part, references to objects are encoded as offsets to the 
start of the Package (*not* the start of the Part!). When installing a Part,
the Package Reader will replce the offsets with the actual address. If the Part
location is known, object references are stored as the final address. This is
done by the `rex` shell app for Packages inside a ROM Extension (REx).

NOS Objects and Refs are described in "Newton Formats Version 1.1". In NOS
Parts, Objects must be aligned correctly. In Package version 0, they are 8 byte 
aligned to the start of the Part (*not* the Package). The padding between
Objects is filled with random bytes. In version 1, if the 0 bit of the second
word is set, alignment is 4 bytes, padded with `0xbf` for NTK packages. NewtEd
seems to use `0x00` for padding.

Frames use arrays to map symbol names to slot values. They are rarely used, 
but there are some references to SuperMaps.

The same Symbol may be defined more than once in a Part 
("/Users/matt/Azureus/unna/games/ChessPadLT/CPlt.pkg").
