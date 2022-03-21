# ino
#### Wrapper script for building with `arduino-cli`

```
% ino -H
ino version 0.2.0 (20 Mar 2022 15:42:04 CDT)

╒══╡ USAGE ╞═══════════════════════════════════════════════════════════════════╕

  Build a sketch:
    ino [flags] [sketch]

  Flash a sketch:
    ino -p /target/device [flags] [sketch]

  Call arduino-cli:
    ino cli [...]

╞══╡ DESCRIPTION ╞═════════════════════════════════════════════════════════════╡

             ──────────────── AUTO-CONFIGURATION ─────────────────

  No arguments are required if configuration can be determined automatically:

    - Sketch exists in current working directory ($PWD)
    - There exists a subdirectory ".fqbn" containing file(s) whose name is
      equal to the Arduino FQBN of desired target(s).

             ──────────────────── SKETCH PATH ────────────────────

  If not called from a sketch root directory, the sketch must be specified as
  argument. If the path is given as argument, it overrides any sketch discovered
  in the current working directory. Sketch path arguments may be paths to either
  the sketch file (*.ino) or the parent directory of that sketch file.

             ───────── FULLY-QUALIFIED BOARD NAME (FQBN) ─────────

  The ".fqbn" directory must be in the root of the resolved sketch directory if
  FQBN is not specified with any of the following methods:

    - Specified directly with flag -b
    - Defined in a shell environment script specified with flag -e

  Both of these will override any FQBNs discovered in the ".fqbn" subdirectory.

             ──────── ARGUMENTS FORWARDED TO ARDUINO-CLI ─────────

  Any unrecognized flags (arguments with leading "-"), along with any remaining
  non-sketch-path arguments, are passed to arduino-cli verbatim.

  For configuration variables, see:
    https://arduino.github.io/arduino-cli/latest/configuration/
    https://arduino.github.io/arduino-cli/latest/platform-specification/

  Alternatively, if the first non-flag argument provided is "cli" (instead of a
  sketch path), all flags and trailing arguments are passed to arduino-cli
  verbatim. In other words, "ino cli" is an alias for "arduino-cli".

╞══╡ FLAGS ╞═══════════════════════════════════════════════════════════════════╡

  -b FQBN           Use FQBN specified at command-line
  -B [FILTER]       List all known FQBNs matching FILTER
  -c                Clean build directory
  -e PATH           Source env file at PATH
  -g                Optimize for debugging
  -h                Brief help summary
  -H                Detailed usage and examples
  -l LEVEL          Set log verbosity to LEVEL (see ino cli --help)
  -p PORT           Upload to device at path PORT
  -*                Append arbitrary flag

╞══╡ EXAMPLES ╞════════════════════════════════════════════════════════════════╡

  A simple "blinky" Arduino project structure might look like the following,
  where you have a sketch source file "blinky.ino" whose basename "blinky" is
  identical to the name of its parent directory:

    blinky/
    └── blinky.ino

  The aim of this script is to reduce the overall typing and complexity of the
  arduino-cli command-line options. Part of the issue is the esoteric and
  rather inconsistent FQBNs.

    ╭─────────────────────────────────────────────────────────────────────╮
    │ [TIP] Create a database of all FQBN files your arduino-cli runtime  │
    │       supports for use with this script. The following command will │
    │       create one file for each FQBN in "/path/to/fqbn.db":          │
    │                                                                     │
    │         % ino -B | xargs -I{} touch /path/to/fqbn.db/{}             │
    │                                                                     │
    │       Then you can simply copy the desired targets into a sketch    │
    │       directory, or delete them from your sketch when not needed.   │
    ╰─────────────────────────────────────────────────────────────────────╯

  First, we create a subdirectory ".fqbn" containing three target devices we
  want to build the executable for:

    blinky/
    ├── .fqbn/
    │   ├── adafruit:nrf52:cluenrf52840
    │   ├── adafruit:samd:adafruit_grandcentral_m4
    │   └── teensy:avr:teensy40
    └── blinky.ino

  Next we can build and export the executables to the "build" subdirectory of
  our sketch. We will add the clean flag (-c) to ensure the Arduino core is
  rebuilt each time, debug flag (-g) so that we can do source-level debugging
  with a hardware debug probe, and extremely verbose logging (-l debug) to see
  what errors were printed if compilation fails:

	  % ino -c -g -l debug

  Since all three targets build without error, I now have the following files
  and directory structure in my sketch project:

    blinky/
    ├── build/
    │   ├── adafruit.nrf52.cluenrf52840/
    │   │   ├── blinky.ino.elf*
    │   │   ├── blinky.ino.hex
    │   │   ├── blinky.ino.map
    │   │   └── blinky.ino.zip
    │   ├── adafruit.samd.adafruit_grandcentral_m4/
    │   │   ├── blinky.ino.bin*
    │   │   ├── blinky.ino.elf*
    │   │   ├── blinky.ino.hex
    │   │   └── blinky.ino.map
    │   └── teensy.avr.teensy40/
    │       ├── blinky.ino.eep
    │       ├── blinky.ino.elf*
    │       ├── blinky.ino.hex
    │       └── blinky.ino.sym
    ├── .fqbn/
    │   ├── adafruit:nrf52:cluenrf52840
    │   ├── adafruit:samd:adafruit_grandcentral_m4
    │   └── teensy:avr:teensy40
    └── blinky.ino

  If we have one of these targets connected and ready to be programmed, let's
  say the Grand Central M4, which appears as USB CDC-ACM device "/dev/ttyACM2",
  then we can override the content of the ".fqbn" directory and specify an FQBN
  explicitly along with the upload port to use:

    % ino -b $( ino -B grandcentral ) -p /dev/ttyACM2

  Along with the mentioned flags, arduino-cli can also read configuration data
  from environment variables as well as its own command-line flags.

  The following example demonstrates an elaborate method to exercise each of
  these capabilities. The calling environment is modified, an environment file
  is given as argument (using process substitution, but you could of course use
  a regular file path), and unrecognized flags are passed on to arduino-cli:

    % ARDUINO_LOGGING_FILE=build.log \
          ino -e <( env -i FQBN=$( ino -B grandcentral ) ) \
              --build-property "build.extra_flags=\"-DFOO=\"BAR\"\""
```
