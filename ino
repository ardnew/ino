#!/usr/bin/env bash

# Verify bash is new enough to support all script features.
if [[ -z $BASH_VERSINFO || ${BASH_VERSINFO[0]} -lt 5 ]]; then
	printf -- "error: bash version 5.0 or newer required (found: %s)\n" \
		"${BASH_VERSION:-<undefined>}"
	exit 127
fi

# Bail on error.
set -oo errexit pipefail

self=${0##*/}
version="0.4.0"
pkgdate="2023-06-10 06:05:52 CST"
summary="${self} version ${version} (${pkgdate})"

# Path relative to sketch source (.ino) files where build artifacts are output.
output="build"

# Save the current datetime to be reused throughout the script.
# Use function `now` to retrieve this time in a desired format.
#  !! Be sure to leave this stamp unformatted so that calls to `now` will know
#     how to parse the source datetime string. !!
__now=$( date )

# now returns a datetime stamp in the given format, but all stamps refer to the
# same instance in time (${__now}).
# If given, $2 will override the instance in time as well.
now() {
	# Use the default format if no format was given.
	[[ ${#} -gt 0 ]] || { echo "${__now}"; return 0; }
	# Unfortunately there are (at least) two widespread and very incompatible
	# versions of the `date` utility:
	if date --version 2>&1 | grep -qi 'gnu'; then
		# 1. GNU coreutils (Linux, Cygwin/MinGW)
		#   (Tries its best to identify input format automatically)
		date --date="${2:-${__now}}" "+${1}"
	else
		# 2. AT&T UNIX (FreeBSD, macOS)
		#   (Requires an input format be specified along with output format)
		infmt="%a %b %d %T %Z %Y" # taken from EXAMPLES section in date(1) manpage
		date -j -f "${infmt}" "${2:-${__now}}" "+${1}"
	fi
}

flags="
  -A PATTERN        Add all FQBNs matching PATTERN to project
  -b FQBN           Use FQBN specified at command-line
  -B [PATTERN]      List all FQBNs matching PATTERN
  -c                Clean build directory
  -D NAME[=VALUE]   Define temporary global macro NAME, optionally with VALUE
  -e PATH           Source env file at PATH
  -g                Optimize for debugging
  -h                Brief help summary
  -H                Detailed usage and examples
  -I PATTERN        Find and install (Git clone) each library matching PATTERN
  -l LEVEL          Set log verbosity to LEVEL (see ${self} cli --help)
  -m                Verify compilation, do not upload sketch
  -n TEMPLATE       Create a sketch .ino (\"empty\", \"blinky\", or file path)
  -p PORT           Upload to device at path PORT
  -R PATTERN        Remove all FQBNs matching PATTERN from project
  -v                Synonym for -l LEVEL (multiple occurrences increases LEVEL)
  -*                Append arbitrary flag
  =PATTERN          Use only FQBNs in project matching PATTERN
  ^PATTERN          Use only FQBNs in project not matching PATTERN
"

brief() {
	cat << _help_
${summary}

USAGE:
 ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓ 
 ┃ ${self} [flags] [sketch]          ╠═╤═Þ build/flash a sketch         ┃ 
 ┃   ${self} =[pattern] [...]        ║ ├───• include only matching FQBN ┃ █
 ┃   ${self} ^[pattern] [...]        ║ └───• exclude all matching FQBN  ┃ █
 ┃ ${self} -B [pattern]              ╠═╤═Þ list/search for FQBN         ┃ █
 ┃   ${self} -A [pattern]            ║ ├───• add FQBN to project        ┃ █
 ┃   ${self} -R [pattern]            ║ └───• remove FQBN from project   ┃ █
 ┃ ${self} cli [...]                 ╚═══Þ alias for arduino-cli        ┃ █
 ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ █
  ▝▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀

FLAGS:
${flags}
_help_
}

usage() {
	cat << _usage_
${summary}

╒══╡ USAGE ╞═══════════════════════════════════════════════════════════════════╕

  ( Arguments in square brackets "[]" are optional. )

  Create an empty sketch:
    ${self} -n empty [sketch]

  List/find an FQBN:
    ${self} -B [pattern]

  Initialize/add a target FQBN:
    ${self} -A pattern [sketch]

  Build a sketch:
    ${self} [flags] [sketch]

  Build a sketch with config/metadata embedded in global macro definitions
    ${self} -D DEBUG -D "BUILD_DATE=\$(date +%s)" [flags] [sketch]

  Build a sketch for a certain target FQBN:
    ${self} =pattern [flags] [sketch]

  Flash a sketch:
    ${self} -p /target/device [flags] [sketch]

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
  arguments are appended for all arduino-cli commands.

  If the FQBN file is empty, no target-specific flags or arguments are
  automatically added for any arduino-cli command.

  An example FQBN file may look like the following:

    % cat .fqbn/adafruit\:avr\:metro
    {
        "global": [ "--format=text", "--log-format=json", "--log-level=trace" ],
        "compile": [ "--build-property=build.extra_flags=-DDEBUG" ]
    }

  In this case, the flags "--format=text" and "--log-format=json" will be added
  to all invocation of arduino-cli for the target FQBN "adafruit:avr:metro".

  If the arduino-cli "compile" command is called (the default command), then
  the flag "--build-property=build.extra_flags=-DFOO" will also be added (in
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
    │       Then you can add any special configurations or flags to reuse │
    │       on every sketch in which these FQBNs are targeted by simply   │
    │       copying/symlinking the respective file into your sketch.      │
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

    ╭─────────────────────────────────────────────────────────────────────╮
    │ [NOTE] You can temporarily disable an FQBN, without deleting or     │
    │        losing its configuration, by prepending a period "." to its  │
    │        file name:                                                   │
    │                                                                     │
    │          % cp .fqbn/{,.}teensy:avr:teensy40     # disable teensy40  │
    │          % cp .fqbn/{.,}teensy:avr:teensy40     # reenable          │
    │                                                                     │
    │        ${self} ignores all hidden files.                                │
    │                                                                     │
    │        Alternatively, you can select/exclude specific FQBNs using   │
    │        the following syntax (refer to "blinky" project above):      │
    │                                                                     │
    │          % ${self} =teensy     # build only teensy:avr:teensy40         │
    │          % ${self} ^teensy     # exclude only teensy:avr:teensy40       │
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
	these capabilities (i.e., please do not do this — there are much simpler 
	alternatives; this is only a demo):

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

# Initialize and return the full path to a temporary file containing the sketch
# file's (.ino) main source. The contents are generated based on the TEMPLATE
# argument given with command-line flag -n, which may be:
#   a.) A filepath that refers to an existing file (regular file or symlink)
#   b.) The name of a predefined template embedded in this script
sketch-template() {
	local name=${1}
	local tmpl=${2}
	if temp=$( mktemp --tmpdir --quiet "${self}.XXXXXX" ); then
		# If given template (via command-line flag -n) is a file path, use it to
		# initialize our sketch.
		if [[ -f "${tmpl}" || -L "${tmpl}" ]] && [[ -r "${tmpl}" ]]; then
			# Copy the given file to a temporary location so that the caller doesn't
			# have to care about where the template comes from.
			cp "${tmpl}" "${temp}" || return 1
		else
			# Otherwise, the given template must be one of the named templates
			# predefined in this script:
			unset -v src
			head="// ${name}.ino (automatically generated $(now '%Y-%m-%d %H:%M:%S'))"
			case "${tmpl,,}" in
				empty)
					src="
void setup() {
}

void loop() {
}
"
					;;
				blink*)
					src="
// Return type of Arduino core function millis()
typedef unsigned long duration_t;

// Turn LED on for ONDUTY ms every PERIOD ms
static duration_t const PERIOD = 1000UL; // 1.0 s
static duration_t const ONDUTY =  100UL; // 0.1 s

// Target device wiring configuration
#define PIN_LED LED_BUILTIN
#define LED_ON  HIGH
#define LED_OFF LOW

void setup() {
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LED_OFF);
}

void loop() {
  // Static data retained across function calls
  static duration_t sync = 0UL;
  static bool       isOn = false;
  // Data that is computed every function call
  duration_t now = millis();
  duration_t ela = now - sync;
  if (isOn) {
    if (ela >= ONDUTY) {
      digitalWrite(PIN_LED, LED_OFF);
      isOn = false;
    }
  } else {
    if (ela >= PERIOD) {
      digitalWrite(PIN_LED, LED_ON);
      isOn = true;
      sync = now;
    }
  }
}
"
			esac
			# Verify we recognized the given template identifier.
			[[ -n ${src} ]] || return 1
			# Write sketch source and return the full path to the temp file if
			# successful. The caller is responsible for cleaning up the temp file.
			if printf '%s\n%s' "${head}" "${src}" > "${temp}"; then
				echo "${temp}"
				return 0
			fi
		fi
	fi
	# Failed to create temporary file
	return 1
}

sketch-init() {
	local inotmpl=${1}
	local -n outp=${2}
	local -n argv=${3}
	for i in "${!argv[@]}"; do
		# Verify given arg contains only valid path chars
		a=$( realpath -qsm "${argv[${i}]}" ) || continue
		# Discard filename if given sketch path is not a directory
		[[ -d "${a}" ]] || a=${a%/*}
		# Ensure sketch directory path exists
		mkdir -p "${a}" || return 1
		# Use a file named "<foo>/<foo>.ino"
		if p=$( realpath -qsm "${a}/${a##*/}.ino" ); then
			if [[ -f "${p}" ]]; then
				err="file exists"
			else
				mv "$( sketch-template "${a##*/}" "${inotmpl}" )" "${p}" ||
					err="invalid template"
			fi
			echo "++ ${a}/${a##*/}.ino${err:+" (failed: ${err})"}"
			if [[ -z ${err} ]]; then
				# Remove this element from the arg array and set sketch path
				unset -v argv[${i}]
				outp=${p}
				return 0
			fi
		fi
	done
	return 1
}

# Normalize the sketch path as the absolute path to the .ino sketch file.
# Can be deduced given either a directory or file path.
# Sketches named <foo>.ino must be inside of a directory named <foo>.
sketch-path() {
	local -n outp=${1}
	local -n argv=${2}
	for i in "${!argv[@]}"; do
		# Verify given path exists
		a=$( realpath -qse "${argv[${i}]}" ) || continue
		# Discard filename if given sketch path is not a directory
		[[ -d "${a}" ]] || a=${a%/*}
		# Verify we have a file named "<foo>/<foo>.ino"
		if p=$( realpath -qse "${a}/${a##*/}.ino" ); then
			# Remove this element from the arg array and set sketch path
			unset -v argv[${i}]
			outp=${p}
			return 0
		fi
	done
	return 1
}

touch-fqbn() {
	unset -v err
	if [[ -f "${1}" && -r "${1}" ]]; then
		echo " ~ ${1##*/}"
	else
		local outpath="${output}/${1##*/}"
		err=$( cat <<__json__ 2>&1 >"${1}"
{
    "global": [
        "--format=text",
        "--log-format=json"
    ],
    "compile": [
        "--warnings=all",
        "--build-cache-path=${outpath//:/\/}",
        "--build-path=${outpath//:/\/}",
        "--dump-profile"
    ]
}
__json__
		)
		echo "++ ${1##*/}${err:+" (failed: ${err})"}"
	fi
	[[ -z ${err} ]]
}

rm-fqbn() {
	unset -v err
	if [[ -f "${1}" && -r "${1}" ]]; then
		err=$( rm "${1}" 2>&1 )
		echo "-- ${1##*/}${err:+" (failed: ${err})"}"
	fi
	[[ -z ${err} ]]
}

split-fqbn() {
	# split FQBN after each colon, verify we have 3 components
	local fqbn=${1}
	local part=( ${fqbn//:/ } )
	local vendor=${part[0]} arch=${part[1]} board=${part[2]} option=${part[3]}
	[[ -z ${vendor} || -z ${arch} || -z ${board} ]] && 
		return 1
	# print board here if an options string is given. otherwise it will be 
	# printed below as the last component.
	printf "%s\n%s\n%s" "${vendor}" "${arch}" "${option:+$'${board}\n'}"
	# last component is the options string if given, and board otherwise.
	local last=${option:-"${board}"}
	# build scheme is delimited with "~" on the FQBN's last component.
	local scheme=${last##*~}
	# keep the "~" prefixed to scheme, so that we can distinguish it in the
	# case where the options string was not given.
	printf "%s\n%s" "${last%~*}" "${scheme:+$'~${scheme}\n'}"
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

# Source the given file containing environment definitions.
source-env() {
	# temporarily set the allexport option if the user does not have it enabled.
	# otherwise, the user has it enabled, leave it on and do not disable it after
	# sourcing the environment file.
	#
	# because of the nature of this function (i.e., modifying shell env), we can't
	# simply do this in a subshell, such as:
	#   ( set -a; ...; )
	# since — by design — subshells cannot modify an ancestor's environment.

	# ensure the flag does not exist for the -n tests below to work
	unset -v set_a

	# check if the user already has allexport enabled. if not, then set the flag
	# to indicate we need to both enable and disable allexport.
	[[ ${-} == *a* ]] || set_a=1

	# if the flag has any definition at all, enable allexport.
	# otherwise, the flag is already set by the user.
	[[ -n ${set_a+?} ]] && set -o allexport

	for src; do command . "${src}"; done

	# if the flag has any definition at all, disable allexport.
	# otherwise, the flag is already set by the user and we do not want to modify
	# their environment unintentionally.
	[[ -n ${set_a+?} ]] && set +o allexport
}

# Parse FQBN file (JSON) for command-line arguments to forward to arduino-cli
fqbn-flags() {
	[[ ${#} -gt 3 && -r "${4}" ]] || return
	local -n glo=${1}
	local -n sel=${2}
	local -a key=( $( command jq -r 'keys[]' "${4}" ) )
	for k in "${key[@]}"; do
		local arg
		while read -re arg; do
			[[ -n ${arg} && ${arg} != null ]] || continue
			case "${k}" in
				global) glo+=( "${arg}" ) ;;
				${3})   sel+=( "${arg}" ) ;;
			esac
		done < <( command jq -r ".${k}[]" "${4}" )
	done
}

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

# Verify we have an arduino-cli executable in $PATH.
declare -x bin
if bin=$( type -P arduino-cli ); then
	bin=$( realpath -qe "${bin}" )
else
	halt "command not found: arduino-cli"
fi

# Before we process the command-line, check if everything should instead be
# forwarded to arduino-cli
if [[ ${#} -gt 0 && ${1} == "cli" ]]; then
	# Replace this process with a call to arduino-cli.
	exec "${bin}" "${@:2}"
fi

# Storage for our command-line arguments and optional flags
declare -a arg flag
declare -x path
declare -x opt_b opt_B opt_c opt_e opt_g opt_l opt_n opt_m opt_o opt_p opt_v
declare -ax opt_sel opt_ign
declare -ax opt_A opt_D opt_I opt_R

# Poor-man's command-line option parsing
while [[ ${#} -gt 0 ]]; do
	case "${1}" in
		-A) shift; optarr opt_A "${@}" ;;   # add all matching FQBNs
		-b) shift; optstr opt_b "${@}" ;;   # use FQBN specified at command-line
		-B) shift; list-all "${@}" ;;       # list all matching FQBNs
		-c) opt_c=1 ;;                      # clean build directory
		-D) shift; optarr opt_D "${1}" ;;   # define all given macros globally
		-e) shift; optstr opt_e "${@}" ;;   # source given env
		-g) opt_g=1 ;;                      # keep debug symbols
		-h) brief; exit 0 ;;                # print brief help and exit
		-H) usage; exit 0 ;;                # print detailed usage and exit
		-I) shift; optarr opt_I "${@}" ;;   # find, install all matching libraries
		-l) shift; optstr opt_l "${@}" ;;   # set log level
		-m) opt_m=1 ;;                      # verify compilation, do not upload
		-n) shift; optstr opt_n "${@}" ;;   # create sketch file (.ino)
		-o) shift; optstr opt_o "${@}" ;;   # build output path
		-p) shift; optstr opt_p "${@}" ;;   # upload to port
		-R) shift; optarr opt_R "${@}" ;;   # remove all matching FQBNs
		-v) opt_v=$(( opt_v + 1 )) ;;       # increase log LEVEL
		=*) opt_sel+=( "${1/#=/}" ) ;;      # use only matching FQBN
		^*) opt_ign+=( "${1/#^/}" ) ;;      # use only not matching FQBN
		--help)                             # capture --help unless its forwarded
			[[ ${#flag[@]} -eq 0 && ${#arg[@]} -eq 0 ]] && 
				{ usage; exit 0; }
				;&                              # fallthrough (requires bash >=4.0)
		-*) flag+=( "${1}" ) ;;             # append arbitrary flag	
		--) flag+=( "${@}" ); break ;;      # append all remaining flags/arguments
		*)  arg+=( "${1}" ) ;;              # append arbitrary argument
	esac
	shift
done

# First install any requested libraries
if [[ ${#opt_I[@]} -gt 0 ]]; then
	for lp in "${opt_I[@]}"; do
		cmd=( lib search "${lp}" )
		#if temp=$( mktemp --quiet "${self}.XXXXXX" ); then	
		#fi
	done
fi

# Determine path to our sketch file
unset -v created
if [[ -n ${opt_n} ]]; then
	# Use PWD if no args given
	[[ ${#arg[@]} -gt 0 ]] || arg+=( "${PWD}" )
	sketch-init "${opt_n}" path arg ||
		halt 'failed to create sketch'
	# When creating a sketch, just like adding/removing FQBNs, we exit early
	# before building so the user can review what occurred.
	# - We don't want to exit immediately though, wait until we have processed
	#   each of these operations requested via command-line flags.
	created=1
elif ! sketch-path path arg; then
	cwd=( "${PWD}" )
	sketch-path path cwd || [[ ${#arg[@]} -gt 0 ]] ||
		halt 'sketch not found'
fi

# Define our output root path
output=${opt_o:-"${path%/*}/${output}"}

# Add all matching FQBNs to project
if [[ ${#opt_A[@]} -gt 0 ]]; then
	fqbndir="${path%/*}/.fqbn"
	mkdir -p "${fqbndir}"
	while read -re fqbn; do
		touch-fqbn "${fqbndir}/${fqbn}"
	done < <( match-fqbn "${opt_A[@]}" )
	exit
fi

# Remove all matching FQBNs from project
if [[ ${#opt_R[@]} -gt 0 ]]; then
	fqbndir="${path%/*}/.fqbn"
	if [[ -d "${fqbndir}" ]]; then
		while read -re fqbn; do
			rm-fqbn "${fqbndir}/${fqbn}"
		done < <( match-fqbn "${opt_R[@]}" )
		# Remove the .fqbn directory if and only if it is empty.
		rmdir "${fqbndir}" &>/dev/null
	fi
	exit
fi

# Don't build the sketch if we just now created it.
[[ -z ${created} ]] || exit

# If given, source the environment file specified with -e <path>.
[[ -n ${opt_e} ]] && source-env "${opt_e}"

#set -o xtrace

# Build all targets discovered by either command-line or .fqbn subdirectory
declare -A target

if [[ -z ${opt_b} ]]; then
	# Check environment for FQBN if not given at command-line.
	[[ -n ${FQBN} ]] && target[${FQBN}]=
else 
	# Always use FQBN specified at command-line, if given.
	target[${opt_b}]=
fi

# Filter the selected FQBNs based on the include/exclude rules given on the
# command-line. 
# Also filters any hidden FQBN files (file names prefixed with ".").
select-fqbn() {
	# Always exclude hidden files
	[[ ${1:-"."} == .* ]] && return 1
	# Exclude files given as exclude patterns
	for ign in "${opt_ign[@]}"; do
		[[ "${1}" =~ ${ign} ]] && return 1
	done
	# Include if we match any given include pattern
	for sel in "${opt_sel[@]}"; do
		[[ "${1}" =~ ${sel} ]] && return 0
	done
	# Otherwise, include the file if no include patterns given
	return ${#opt_sel[@]} # (success iff opt_sel[] is empty)
}

# If FQBN not found in command-line or environment, check .fqbn subdirectory
if [[ ${#target[@]} -eq 0 ]] && [[ -d "${path%/*}/.fqbn" ]]; then
	while read -re f; do
		# Ignore hidden and excluded files
		select-fqbn "${f}" && target[${f}]="${path%/*}/.fqbn/${f}"
	done < <( command ls -t "${path%/*}/.fqbn" )
fi

# Verify we have a board specified
[[ ${#target[@]} -gt 0 ]] || [[ ${#arg[@]} -gt 0 ]] ||
	halt 'fully-qualified board name (FQBN) undefined'

# Build every FQBN found in target array
for fqbn in "${!target[@]}"; do
	unset -v cmd part base option scheme
	# build the command string based on given arguments
	cmd=( "${bin}" )
	if [[ ${#arg[@]} -gt 0 ]]; then
		cmd+=( ${arg[@]} )
	else
		part=( $( split-fqbn "${fqbn}" ) ) || 
			halt "invalid fully-qualified board name (FQBN): ${fqbn}"
		base=$( join-str ":" "${part[@]::3}" )
		[[ "${part[3]:0:1}" == "~" ]] && 
			option=           scheme=${part[3]:1} ||
			option=${part[3]} scheme=${part[4]:1}
		# spec always contains "vendor:arch:board" at minimum; but if options were 
		# given, ":options" is appended.
		spec="${base}${option:+":${option}"}"
		# if scheme was given, place the files in a respectively named subdirectory
		out="${output}/${base}/${scheme:+"${scheme}/"}$( now '%Y-%m-%d %H-%M-%S' )"
		cmd+=( 
			compile 
			--fqbn="${base}${option:+":${option}"}" 
			--export-binaries 
			--output-dir="${out//:/\/}" 
		)
		[[ ${#opt_D[@]} -eq 0 ]] || cmd+=( --build-property=build.extra_flags=${opt_D[@]/#/-D} )
		[[ -z ${opt_l} ]] || cmd+=( --log-level="${opt_l}" --verbose )
		[[ -z ${opt_c} ]] || cmd+=( --clean )
		[[ -z ${opt_g} ]] || cmd+=( --optimize-for-debug )
		# Flash target unless we are verifying compilation or no port specified.
		[[ -n ${opt_m} || -z ${opt_p} ]] || cmd+=( --port="${opt_p}" --upload )
	fi
	[[ ${#flag[@]} -eq 0 ]] || cmd+=( ${flag[@]} )
	# Parse the FQBN-specific flags from the JSON-formatted file. Files may be
	# further discriminated by Arduino build options and/or ino build scheme.
	declare -a global=() selcmd=()
	fqbn-flags global selcmd "${cmd[1]}" "${target[${fqbn}]}" &&
		# Use only the global flags and those defined for our selected command
		cmd+=( "${global[@]}" "${selcmd[@]}" )
	# Append sketch path to command-line only if we are running a command that
	# expects to receive a path
	[[ ${cmd[1]} =~ compile|debug|sketch|upload ]] &&
		cmd+=( "${path%/*}" )
	# Run command in a subshell
	set -o xtrace
	"${cmd[@]}"
	set +o xtrace
	# Keep a snapshot of the sketch. Thank me later.
	[[ -d "${output}/${fqbn}/sketch" && -d "${out}" ]] &&
		cp -r "${output}/${fqbn}/sketch" "${out}"
done
