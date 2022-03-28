#!/usr/bin/env bash

# ------------------------------------------------------------------------------
#
#
# ------------------------------------------------------------------------------

# Bail on error.
set -oo errexit pipefail

self=${0##*/}
version="0.3.0"
pkgdate="28 Mar 2022 10:20:04 CDT"
summary="${self} version ${version} (${pkgdate})"

flags="
  -A [PATTERN]      Add all FQBNs matching PATTERN to project
  -b FQBN           Use FQBN specified at command-line
  -B [PATTERN]      List all FQBNs matching PATTERN
  -c                Clean build directory
  -e PATH           Source env file at PATH
  -g                Optimize for debugging
  -h                Brief help summary
  -H                Detailed usage and examples
  -l LEVEL          Set log verbosity to LEVEL (see ${self} cli --help)
  -p PORT           Upload to device at path PORT
  -R [PATTERN]      Remove all FQBNs matching PATTERN from project
  -*                Append arbitrary flag
"

brief() {
	cat << _help_
${summary}

USAGE:
  ${self} [flags] [sketch]         # build/flash a sketch
  ${self} -B [pattern]             # list/search for FQBN
  ${self} cli [...]                # alias for arduino-cli

FLAGS:
${flags}
_help_
}

usage() {
	cat << _usage_
${summary}

╒══╡ USAGE ╞═══════════════════════════════════════════════════════════════════╕

  Build a sketch:
    ${self} [flags] [sketch]

  Flash a sketch:
    ${self} -p /target/device [flags] [sketch]

  List/find an FQBN:
    ino -B [pattern]

  Call arduino-cli:
    ${self} cli [...]

╞══╡ DESCRIPTION ╞═════════════════════════════════════════════════════════════╡

             ──────────────── AUTO-CONFIGURATION ─────────────────

  No arguments are required if configuration can be determined automatically:

    - Sketch exists in current working directory (\$PWD)
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

        ${self} upload -i INPUT-FILE               # WRONG
        ${self} upload --input-file INPUT-FILE     # WRONG
        ${self} upload --input-file=INPUT-FILE     # OK!
        ${self} upload -- -i INPUT-FILE            # OK!
        ${self} cli upload -i INPUT-FILE           # OK!

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
${flags}
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
    │         % ${self} -B | xargs -I{} touch /path/to/fqbn.db/{}             │
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
    │          % ${self} -A cluenrf52 grandcentral teensy40                   │
    │                                                                     │
    │        This creates the same directory tree as listed above.        │
    │                                                                     │
    │        Using the corresponding remove flag (-R pattern) will remove │
    │        all matching FQBNs from the project:                         │
    │                                                                     │
    │          % ${self} -R cluenrf52 grandcentral teensy40                   │
    │                                                                     │
    ╰─────────────────────────────────────────────────────────────────────╯

  Next we can build and export the executables to the "build" subdirectory of
  our sketch. We will add the clean flag (-c) to ensure the Arduino core is
  rebuilt each time, debug flag (-g) so that we can do source-level debugging
  with a hardware debug probe, and extremely verbose logging (-l debug) to see 
  what errors were printed if compilation fails:

	  % ${self} -c -g -l debug

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

    % ${self} -b \$( ino -B grandcentral ) -p /dev/ttyACM2

  Along with the mentioned flags, arduino-cli can also read configuration data
  from environment variables as well as its own command-line flags. 

  The following example demonstrates an elaborate method to exercise each of
  these capabilities: 

    % ARDUINO_LOGGING_FILE=build.log \\
        ${self} -e <( env -i FQBN=\$( ino -B grandcentral ) ) \\
          --build-property "build.extra_flags=\"-DFOO=\"BAR\"\""

  Specifically, the following behaviors are demonstrated:

    a. Modify ARDUINO_LOGGING_FILE in caller environment
    b. Source environment "file", provided as flag -e argument
      i. Bash's process substitution "<( cmd )" is used to create a
         virtual file, but you could of course use a regular file path
    c. Unrecognized flags are passed on to arduino-cli verbatim
_usage_
}

# Bail on demand.
halt() { printf -- 'error: %s\n' "${@}" >&2; exit 1; }

declare -x bin
if bin=$( type -P arduino-cli ); then
	bin=$( realpath -qe "${bin}" )
else
	halt "command not found: arduino-cli"
fi

# Normalize the sketch path as the absolute path to the .ino sketch file.
# Can be deduced given either a directory or file path. 
# Sketches named <foo>.ino must be inside of a directory named <foo>.
sketch-path() {
	local -n _path=${1}
	local -n _arg=${2}
	for i in "${!_arg[@]}"; do
		# Verify given path exists
		a=$( realpath -qse "${_arg[${i}]}" ) || continue
		# Discard filename if given sketch path is not a directory
		[[ -d "${a}" ]] || a=${a%/*}
		# Verify we have a file named "<foo>/<foo>.ino"
		if p=$( realpath -qse "${a}/${a##*/}.ino" ); then
			# Remove this element from the arg array and set sketch path
			unset -v _arg[${i}]
			_path=${p}
			return 0
		fi
	done
	return 1
}

valid-fqbn() {
	# split FQBN after each colon, verify we have 3 components
  local fqbn=${1}
	local part=( ${fqbn//:/ } )
	local vendor=${part[0]} arch=${part[1]} board=${part[2]}
	if [[ -z ${vendor} ]] || [[ -z ${arch} ]] || [[ -z ${board} ]]; then
		printf -- 'invalid fully-qualified board name (fqbn): %s' "${fqbn}"
		return 1
	fi
}

join-str() {
    local d=${1-} f=${2-};
    shift 2 && printf %s "$f" "${@/#/$d}"
}

match-fqbn() {
	# build the arguments to arduino-cli
	local -a cmd=( "${bin}" board listall )
	# filter the output to display only the FQBNs
	local -a fqbn=( command grep -oP '\S+:\S+:\S+' )
	# filter the FQBNs to those that match any given argument
	local -a grep=( command grep -P "$( join-str "|" "${@}" )" )
	# sort the FQBNs alphabetically
	local -a sort=( command sort -b -d -f )
	# invoke pipeline
	"${cmd[@]}" | "${fqbn[@]}" | "${grep[@]}" | "${sort[@]}"
}

list-all() {
	# stop script after call to arduino-cli
	match-fqbn "${@}"
	exit
}

# Before we process the command-line, check if everything should instead be
# forwarded to arduino-cli.
if [[ ${#} -gt 0 && ${1} == "cli" ]]; then
	# Replace this process ewith a call to arduino-cli.
	exec "${bin}" "${@:2}"
fi

# Storage for our command line arguments and optional flags
declare -a arg flag
declare -x opt_b opt_B opt_c opt_e opt_g opt_l opt_p
declare -ax opt_A opt_R

# Assign $2 to the variable named $1 (or die).
optstr() {
	[[ ${#} -gt 1 ]] || 
		halt "flag requires argument (-${1#opt_})"
	local -n v=${1}
	v=${2}
}

# Append $@ to the array variable named $1 (or die).
optarr() {
	[[ ${#} -gt 1 ]] ||
		halt "flag requires argument (-${1#opt_})"
	local -n v=${1}
	v+=( "${@:2}" )
}

# Poor-man's command-line option parsing
while [[ ${#} -gt 0 ]]; do
	case "${1}" in
		-A) shift; optarr opt_A "${@}" ;;   # add all matching FQBNs
		-b) shift; optstr opt_b "${@}" ;;   # use FQBN specified at command-line
		-B) shift; list-all "${@}" ;;       # list all matching FQBNs
		-c) opt_c=1 ;;                      # clean build directory
		-e) shift; optstr opt_e "${@}" ;;   # source given env
		-g) opt_g=1 ;;                      # keep debug symbols
		-h) brief; exit 0 ;;                # print brief help and exit
		-H) usage; exit 0 ;;                # print detailed usage and exit
		-l) shift; optstr opt_l "${@}" ;;   # set log level
		-p) shift; optstr opt_p "${@}" ;;   # upload to port
		-R) shift; optarr opt_R "${@}" ;;   # remove all matching FQBNs
		-*) flag+=( "${1}" ) ;;             # append arbitrary flag
		--) flag+=( "${@}" ); break ;;      # append all remaining flags/arguments
		*)  arg+=( "${1}" ) ;;              # append arbitrary argument
	esac
	shift
done

# Determine path to our sketch file.
if ! sketch-path path arg; then
	cwd=( "${PWD}" )
	sketch-path path cwd || [[ ${#arg[@]} -gt 0 ]] ||  halt 'sketch not found'
fi

# Keep a flag indicating if we have processed .fqbn content
unset -v fqbnmod
# Add all matching FQBNs to project
if [[ ${#opt_A[@]} -gt 0 ]]; then
	fqbndir="${path%/*}/.fqbn"
	mkdir -p "${fqbndir}"
	while read -re fqbn; do
		if [[ -r "${fqbndir}/${fqbn}" ]]; then
			echo " ~ ${fqbn}"
		else
			err=$( touch "${fqbndir}/${fqbn}" 2>&1 )
			echo "++ ${fqbn}${err:+" (failed: ${err})"}"
		fi
	done < <( match-fqbn "${opt_A[@]}" )
	fqbnmod=1
fi

# Remove all matching FQBNs from project
if [[ ${#opt_R[@]} -gt 0 ]]; then
	fqbndir="${path%/*}/.fqbn"
	if [[ -d "${fqbndir}" ]]; then
		while read -re fqbn; do
			if [[ -r "${fqbndir}/${fqbn}" ]]; then
				err=$( rm "${fqbndir}/${fqbn}" 2>&1 )
				echo "-- ${fqbn}${err:+" (failed: ${err})"}"
			else
				echo " ~ ${fqbn}"
				touch "${fqbndir}/${fqbn}"
			fi
		done < <( match-fqbn "${opt_A[@]}" )
		# Remove the .fqbn directory if and only if it is empty.
		rmdir "${fqbndir}" &>/dev/null
	fi
	fqbnmod=1
fi

fqbn-flags() {
	[[ ${#} -gt 1 && -r "${2}" ]] || return
	local -n cmd=${1}
	local -a key=( $( command jq -r 'keys[]' "${2}" ) ) 
	for k in "${key[@]}"; do
		while read -re arg; do
			[[ -n ${arg} && ${arg} != null ]] &&
				cmd[${k}]+="${arg} "
		done < <( command jq -r ".${k}[]" "${2}" )
	done
}

# Source the given file containing environment definitions.
source-env() {
	# temporarily set the allexport option if the user does not have it enabled.
	# otherwise, the user has it enabled, leave it on and do not disable it after
	# sourcing the environment file.

	# ensure the flag does not exist for the -n tests below to work
	unset -v set_a 

	# check if the user already has allexport enabled. if not, then set the flag
	# to indicate we need to both enable and disable allexport.
	[[ ${-} == *a* ]] || set_a=1 

	# if the flag has any definition at all, enable allexport. 
	# otherwise, the flag is already set by the user.
	[[ -n ${set_a+?} ]] && set -o allexport

	for src; do command . "${src}"; done

	# if the flag has any definition at all, disable allexport
	# otherwise, the flag is already set by the user and we do not want to modify
	# their environment unintentionally.
	[[ -n ${set_a+?} ]] && set +o allexport
}

# If given, source the environment file specified with -e <path>.
[[ -n ${opt_e} ]] && source-env "${opt_e}"

# Build all targets discovered by either command-line or .fqbn subdirectory
declare -a target

if [[ -z ${opt_b} ]]; then
	# Check environment for FQBN if not given at command-line.
	[[ -n ${FQBN} ]] && target+=( "${FQBN}" )
else
	# Always use FQBN specified at command-line, if given.
	target+=( "${opt_b}" )
fi

# If FQBN not specified at command-line or environment, check .fqbn subdirectory
if [[ ${#target[@]} -eq 0 ]] && [[ -d "${path%/*}/.fqbn" ]]; then
	while read -re f; do
		target+=( "${f}" )
	done < <( command ls -t "${path%/*}/.fqbn" )
fi

# Verify we have a board specified
[[ ${#target[@]} -gt 0 ]] || [[ ${#arg[@]} -gt 0 ]] ||
	halt 'fully-qualified board name (fqbn) undefined'

# Build every FQBN found in target array
for fqbn in "${target[@]}"; do
	unset -v cmd
	# build the command string based on given arguments
	cmd=( "${bin}" )
	if [[ ${#arg[@]} -gt 0 ]]; then
		cmd+=( "${arg[@]}" )
	else
		err=$( valid-fqbn "${fqbn}" ) || halt "${err}"
		cmd+=( compile --fqbn "${fqbn}" --export-binaries )
		[[ -z ${opt_l} ]] || cmd+=( --log-level "${opt_l}" --verbose )
		[[ -z ${opt_c} ]] || cmd+=( --clean )
		[[ -z ${opt_g} ]] || cmd+=( --optimize-for-debug )
		[[ -z ${opt_p} ]] || cmd+=( --port "${opt_p}" --upload )
	fi
	[[ ${#flag[@]} -eq 0 ]] || cmd+=( "${flag[@]}" )
	# Parse the FQBN-specific flags from the JSON-formatted file
	declare -A extra
	fqbn-flags extra "${path%/*}/.fqbn/${fqbn}"
	# Use only the global flags and those defined for our selected command
	cmd+=( ${extra['global']} ${extra[${cmd[1]}]} )
	cmd+=( "${path%/*}" )
	# run command
	set -o xtrace
	"${cmd[@]}"
	set +o xtrace
done
