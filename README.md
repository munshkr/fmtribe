# FMTribe #

OPL3 synth/drum machine/thingie focused on live jamming for DOS.

Tested only with DJGPP and a PC with a Sound Blaster-compatible soundcard.

![Step Sequencer](https://i.imgur.com/4EfKrE2.gif)&nbsp;![Instrument Editor](http://i.imgur.com/fkLsV9T.png)

## Download ##

* Install a DPMI server if you don't have one. See [here](https://github.com/munshkr/fmtribe/wiki/Install-DPMI-server) for instructions.
* Go to [releases](https://github.com/munshkr/fmtribe/releases) tab to download
  binaries.
* Unzip and run FMTRIBE.EXE.
* Have fun!

## Usage ##

* `Esc`: Exit
* `F5`: Play / pause
* `F7`: Stop after bar (2 times, stop immeadiately)
* `F9`: Tap tempo
* `Shift` + `F9`: Toggle metronome
* `F10`: Toggle "follow cursor"
* `Left`: Move to previous frame
* `Right`: Move to next frame
* `Up`: Move to previous channel
* `Down`: Move to next channel
* `Del`: Clear pattern for current channel
* `Ctrl` + `Del`: Clear pattern for all channels
* `Tab`: Switch instrument editor for the current channel
* `M`: Toggle "apply step changes to all frames"
* `P`: Toggle "play channel instrument"
* `Z`: Toggle "record steps"
* `Alt` + Number: Mute/unmute channel

### Step Sequencer ###

To toggle a step in the pattern of the 16-step sequencer, use the keys:

    Q W E R T Y U I
    A S D F G H J K

To select one of the eight channels, use keys 1 through 8.
You can mute a channel anytime using `Alt` + NumberKey.

Press `Alt` + StepKey to define microsteps (upto 3 for now).

### Instrument Editor ###

To customize the parameters of an instrument, press the Tab key, select a field
and change its value. The parameters will update automatically if the step
sequencer is playing.

* `Left`/`Right`: Increase/decrease value for the current field
* `Up`/`Down`: Move between parameter fields

Also, you can still change channels with the numbers from the instrument
editor.

To change the base note of the instrument, use the keys:

      W   E       T   Y   U
    A   S   D   F   G   H   J

as in a keyboard of one octave. To change the base octave, use `PageUp`/`PageDown`.

## Source ##

To tinker with the source code, you need to install DJGPP and its C compiler.
Go to the [zip picker](http://www.delorie.com/djgpp/zip-picker.html) and follow
instructions.
