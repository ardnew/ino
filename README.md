# ino
#### Wrapper script for building with `arduino-cli`

## Brief
```
% ino -h
ino version 0.3.0 (28 Mar 2022 10:20:04 CDT)

USAGE:
  ino [flags] [sketch]         # build/flash a sketch
  ino -B [pattern]             # list/search for FQBN
  ino cli [...]                # alias for arduino-cli

FLAGS:

  -A PATTERN        Add all FQBNs matching PATTERN to project
  -b FQBN           Use FQBN specified at command-line
  -B [PATTERN]      List all FQBNs matching PATTERN
  -c                Clean build directory
  -e PATH           Source env file at PATH
  -g                Optimize for debugging
  -h                Brief help summary
  -H                Detailed usage and examples
  -l LEVEL          Set log verbosity to LEVEL (see ino cli --help)
  -p PORT           Upload to device at path PORT
  -R PATTERN        Remove all FQBNs matching PATTERN from project
  -*                Append arbitrary flag
```

## Usage
```
% ino -H
ino version 0.3.0 (28 Mar 2022 10:20:04 CDT)

╒══╡ USAGE ╞═══════════════════════════════════════════════════════════════════╕

  Build a sketch:
    ino [flags] [sketch]

  Flash a sketch:
    ino -p /target/device [flags] [sketch]

  List/find an FQBN:
    ino -B [pattern]

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
  non-sketch-path arguments, are passed to arduino-cli verbatim. All flags given
  after the end-of-arguments delimiter "--" are also passed to arduino-cli.
  
    - Note that any arduino-cli flag intermixed (i.e., preceeding "--") which 
      requires an argument must be specified using its long form syntax 
      (i.e., "--flag=VALUE"). For example: 

        ino upload -i INPUT-FILE               # WRONG
        ino upload --input-file INPUT-FILE     # WRONG
        ino upload --input-file=INPUT-FILE     # OK!
        ino upload -- -i INPUT-FILE            # OK!
        ino cli upload -i INPUT-FILE           # OK!

  For configuration variables, see:
    https://arduino.github.io/arduino-cli/latest/configuration/
    https://arduino.github.io/arduino-cli/latest/platform-specification/

  Alternatively, if the first non-flag argument provided is "cli" (instead of a
  sketch path), all flags and trailing arguments are passed to arduino-cli
  verbatim. In other words, "ino cli" is an alias for "arduino-cli".

         ──────── TARGET-SPECIFIC FLAGS PASSED TO ARDUINO-CLI ─────────         
	
  The FQBN files described above under "FULLY-QUALIFIED BOARD NAME (FQBN)" also
  allow you to store target-specific settings that are used automatically when
  calling arduino-cli for that FQBN. The files are JSON-formatted with a single
  dictionary keyed by arduino-cli commands. The value for each of these keys is
  a list of command-line flags and arguments to append when calling that command
  for that FQBN. An additional "global" key is supported whose list of flags and
  arguments are appended for all arduin-cli commands.

  If the FQBN file is empty, no target-specific flags or arguments are 
  automatically added for any arduino-cli command.

  An example FQBN file may look like the following:

    % cat .fqbn/adafruit\:avr\:metro
    {
        "global": [ "--format text", "--log-format json" ],
        "compile": [ "--build-property 'build.extra_flags=-DFOO'" ]
    }

  In this case, the flags "--format text" and "--log-format json" will be added
  to all invocation of arduino-cli for the target FQBN "adafruit:avr:metro".

  If the arduino-cli "compile" command is called (the default command), then the
  flag "--build-property 'build.extra_flags=-DFOO'" will always be added (in 
  addition to the global flags above) for the target FQBN "adafruit:avr:metro".

╞══╡ FLAGS ╞═══════════════════════════════════════════════════════════════════╡

  -A [PATTERN]      Add all FQBNs matching PATTERN to project
  -b FQBN           Use FQBN specified at command-line
  -B [PATTERN]      List all FQBNs matching PATTERN
  -c                Clean build directory
  -e PATH           Source env file at PATH
  -g                Optimize for debugging
  -h                Brief help summary
  -H                Detailed usage and examples
  -l LEVEL          Set log verbosity to LEVEL (see ino cli --help)
  -p PORT           Upload to device at path PORT
  -R [PATTERN]      Remove all FQBNs matching PATTERN from project
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

    ╭─────────────────────────────────────────────────────────────────────╮
    │ [NOTE] Alternatively, you can use the add flag (-A pattern) to      │
    │        create the ".fqbn" directory in the project, initialized     │
    │        with all matching FQBNs. Run from the project root:          │
    │                                                                     │
    │          % ino -A cluenrf52 grandcentral teensy40                   │
    │                                                                     │
    │        This creates the same directory tree as listed above.        │
    │                                                                     │
    │        Using the corresponding remove flag (-R pattern) will remove │
    │        all matching FQBNs from the project:                         │
    │                                                                     │
    │          % ino -R cluenrf52 grandcentral teensy40                   │
    │                                                                     │
    ╰─────────────────────────────────────────────────────────────────────╯

  Next we can build and export the executables to the "build" subdirectory of
  our sketch. We will add the clean flag (-c) to ensure the Arduino core is
  rebuilt each time, debug flag (-g) so that we can do source-level debugging
  with a hardware debug probe, and extremely verbose logging (-l debug) to see 
  what errors were printed if compilation fails:

	  % ino -c -g -l debug

  Since all three targets build without error, we now have the following files
  and directory structure in our sketch directory:

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
  these capabilities: 

    % ARDUINO_LOGGING_FILE=build.log \
        ino -e <( env -i FQBN=$( ino -B grandcentral ) ) \
          --build-property "build.extra_flags=\"-DFOO=\"BAR\"\""

  Specifically, the following behaviors are demonstrated:

    a. Modify ARDUINO_LOGGING_FILE in caller environment
    b. Source environment "file", provided as flag -e argument
      i. Bash's process substitution "<( cmd )" is used to create a
         virtual file, but you could of course use a regular file path
    c. Unrecognized flags are passed on to arduino-cli verbatim
```
