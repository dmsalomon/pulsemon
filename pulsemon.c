
/**
 * Author: Dov Salomon
 *
 * A C++ program I found online and simplified into a C
 * program. Authors original comments are found below.
 *
 * Author: Jason White
 * License: Public Domain
 *
 * Description:
 * This is a simple test program to hook into PulseAudio volume change
 * notifications. It was created for the possibility of having an automatically
 * updating volume widget in a tiling window manager status bar.
 *
 * Compiling:
 *
 *     g++ $(shell pkg-config libpulse --cflags --libs) pulsetest.c -o pulsetest
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pulse/pulseaudio.h>

pa_context *context;
pa_mainloop_api *mainloop_api;

void quit(int ret)
{
    if (mainloop_api)
        mainloop_api->quit(mainloop_api, ret);
    else
        exit(ret);
}

/* prints message and exit */
void die(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "pulsemon: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (fmt[0] && fmt[strlen(fmt)-1] != ':') {
        fputc('\n', stderr);
    } else {
        fputc(' ', stderr);
        perror(NULL);
    }

    quit(1);
}

void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *data)
{
    double volume;

    if (i) {
        volume = (double)pa_cvolume_avg(&(i->volume)) / (double)PA_VOLUME_NORM;
        printf("%s %.0f%% %s\n", i->name, volume * 100.0f, i->mute ? "muted" :  "unmuted");
    }
}

void server_info_callback(pa_context *c, const pa_server_info *i, void *data)
{
    pa_context_get_sink_info_by_name(c, i->default_sink_name, sink_info_callback, data);
}

void subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *data)
{
    /* show the default sink info always */
    pa_context_get_server_info(c, server_info_callback, data);
}

void context_state_callback(pa_context *c, void *data)
{
    assert(c && data);

    switch (pa_context_get_state(c)) {
    case PA_CONTEXT_READY:
        pa_context_get_server_info(c, server_info_callback, data);
        pa_context_set_subscribe_callback(c, subscribe_callback, data);
        pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SINK_INPUT, NULL, NULL);
        break;
    case PA_CONTEXT_FAILED:
        die("Connection failure: %s", pa_strerror(pa_context_errno(c)));
        break;
    case PA_CONTEXT_TERMINATED:
        quit(0);
        die("PulseAudio connection terminated");
        break;
    default:
        break;
    }
}

int main()
{
    pa_mainloop *m = NULL;

    setlinebuf(stdout);

    if (!(m = pa_mainloop_new()))
        die("pa_mainloop_new():");

    mainloop_api = pa_mainloop_get_api(m);

    if (!(context = pa_context_new(mainloop_api, NULL)))
        die("pa_context_new():");


    if (pa_context_connect(context, NULL, 0, NULL) < 0)
        die("pa_context_connect():");

    pa_context_set_state_callback(context, context_state_callback, mainloop_api);

    if (pa_mainloop_run(m, NULL) < 0)
        die("pa_mainloop_run():");
}
