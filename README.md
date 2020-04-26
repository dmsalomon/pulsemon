# pulsemon
Simple program to monitor the PulseAudio default sink status efficiently.

## usage
```sh
pulsemon
```

## (some) details
_pulsemon_ subscribes to the PulseAudio server, and registers a callback for the status of the default
sink whenever a change to any sink is detected. More specifically, it listens
on PA_SUBSCRIPTION_MASK_SINK and PA_SUBSCRIPTION_MASK_SINK_INPUT, which seemed to cover
all the events I needed (all events can be
[seen here](https://freedesktop.org/software/pulseaudio/doxygen/def_8h.html#a6bedfa147a9565383f1f44642cfef6a3)).

This program basically uses zero cpu and memory, since it is event driven is only runs a change is detected.

Each status update is a line of stdout with 3 space separated fields: the output device, volume percentage, and muted/unmuted.

## example
I use this in my lemonbar like this:

```sh
pulsemon |
while read card vol mute; do
    echo "V${mute} ${vol}"
done |
sed -u -e 's/unmuted//' -e 's/muted//'
```
