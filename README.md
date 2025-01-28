
# newtfmt

"newtfmt" is a tool designed to read NewtonOS Package files, analyze their 
contents, and convert them into an editable text format. Once modified, the 
tool can reconstruct a valid Package file from the text representation.

The primary goal is to enable the conversion of any Package file into one or 
more easily editable files. NOS parts should be written as NewtonScript source 
files, while embedded images and sounds should be exported in standard formats.

This approach allows for a deeper understanding of existing Packages, 
facilitates debugging and modifications, and enables the creation of entirely 
new Newton applications from scratch. Furthermore, unpacking and repacking a 
Package should yield an identical file, ensuring fidelity 
in the conversion process.

## Current Status

The current implementation can read Package files and correctly interpret the 
package header. It generates an ARM32 assembly file, which can be assembled 
into an object file using the GNU assembler. The GNU objcopy tool can then 
extract the .data segment as binary, effectively reconstructing 
the original Package file.

Extracting NewtonScript objects from NOS Parts within the Package works well 
and they are written to the assembly file with labels.

## Next Steps

All data written to the assembly file should utilize labels rather than direct 
numerical indexing. Achieving this will ensure that modifications — such as 
inserting or removing data — do not disrupt the structure of the Package, 
even if addresses and offsets change dynamically.

Fix duplicate symbols that have the same label. `Error: symbol `sym_0_viewFormat' is already defined`

List all subtypes, "book", etc. .  `WARNING: Part Entry 0: unknown type "auto"`

Can't create object file `"/Users/matt/Azureus/unna/applications/calculator/IPCalc1.1/ipcalc.pkg"`

No output at all: `/Users/matt/Azureus/unna/applications/calculator/GoFigure.pkg`

Crash `"/Users/matt/Azureus/unna/applications/Mapper/CA State map v1.0.pkg"`

Find a Watson package!

Despite the long list, we are very far and generate mostly identical files. 
Very nice!

## Long-Term Goals

Generating assembly files serves as a verification step to ensure that all 
aspects of the Package format are correctly understood. Given the existence 
of thousands of Newton Packages in the wild, processing them with "newtfmt" 
should help identify all variations of the format and refine both the 
reader and writer components.

The next objective is to reverse-engineer all Newton Packages into their 
most original and human-readable format. This will require integrating both 
a decompiler and compiler, along with a functional NewtonScript interpreter.

Furthermore, "newtfmt" should also be capable of extracting all 
NewtonScript code from ROM images and translating it into ARM32 assembly. 
This capability would significantly aid efforts to fix bugs within the ROM, 
particularly in light of the looming Year 2040 (Y2K40) time-related issues.

Ultimately, "newtfmt" could evolve into a NewtonOS-compatible environment, 
making it possible to run NewtonScript applications natively 
without requiring an emulator.

— Matthias

