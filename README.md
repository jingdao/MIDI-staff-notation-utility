MIDI-staff-notation-utility
==========================

Parses a Musical Instrument Digital Interface (MIDI) file
and converts it into text representation similiar to 
staff notation.

Notation
--------

`Bb2`: represents notes. e.g. B-flat on the second octave (middle C is C4)

`.`: represents rest

`-`: represents held note

`|`: represents bar line

`()`: represents block chords (notes played simultaneously)

` `: each time division (quarter note for 4/4 time) is separated by a space


Usage
-----

	./configure
	make
	mid2stf your_input.mid your_output.stf

Example output
-------------

	|0                   |1 (C major) t=4/4 !=100 |2                             |3                    |
	|                    |. . . .                 |.        .      .      .      |.  .     .   .       |
	|Acoustic Grand Piano|                        |                              |                     |
	|                    |. . . .                 |.        .      .      .      |.  .     .   .       |
	|Acoustic Grand Piano|                        |                              |                     |
	|                    |. . . .                 |(G4AC5D) (----) (----) (----) |.  .     .   .       |
	|Acoustic Grand Piano|                        |mp                            |                     |
	|                    |. . . .                 |G3       A4G    FD     A3G    |G# B2D3  FBb G#G     |
	|Acoustic Grand Piano|                        |mf                            |                     |
	|                    |. . . .                 |Eb4      Eb-Eb  Eb     Eb-Eb  |Eb Eb-Eb Eb  Eb-Eb   |
	|Drums               |                        |mp                            |                     |

References
----------

http://www.archduke.org/midi/index.html
