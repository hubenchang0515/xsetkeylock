#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int device_listener_error_handler(Display* display, XErrorEvent* event);
static Bool get_key_mask(XkbDescPtr xkb, const char* name, unsigned int* mask);
static Bool set_key_lock(Display* display, const char* key, Bool lock);
static Bool get_key_lock(Display* display, const char* key);
static Bool check_args(int argc, char* argv[]);
static Bool str_in_set(const char* set[], int len, const char* str);

int main(int argc, char* argv[])
{
    if(!check_args(argc, argv))
    {
        return EXIT_FAILURE;
    }

    XSetErrorHandler(device_listener_error_handler);
    Display* display = XOpenDisplay(NULL);
    if(display == NULL)
    {
        fprintf(stderr, "X11/Xlib cannot open display.\n");
        return EXIT_FAILURE;
    }

    int ret;
    if(strcmp(argv[2], "on") == 0)
        ret = set_key_lock(display, argv[1], True);
    else if(strcmp(argv[2], "off") == 0)
        ret = set_key_lock(display, argv[1], False);
    else if(strcmp(argv[2], "toggle") == 0)
        ret = set_key_lock(display, argv[1], !get_key_lock(display, argv[1]));
    else if(strcmp(argv[2], "status") == 0)
        ret = get_key_lock(display, argv[1]);
    XCloseDisplay(display);
    return ret;
}

static int device_listener_error_handler(Display* display, XErrorEvent* event)
{
    char message[1024];
    XGetErrorText(display, event->error_code, message, 1024);
    fprintf(stderr, "X11/Xlib error: %s\n", message);
    return 0;
}

static Bool get_key_mask(XkbDescPtr xkb, const char* name, unsigned int* mask)
{
    for(int i = 0; i < XkbNumVirtualMods; i++)
    {
        if(xkb->names->vmods[i] == 0)
            continue;
        char* modStr = XGetAtomName(xkb->dpy, xkb->names->vmods[i]);
        if(modStr != NULL && strcmp(name, modStr) == 0)
        {
            XkbVirtualModsToReal(xkb, 1<<i, mask);
            return True;
        }
    }

    return False;
}

static Bool set_key_lock(Display* display, const char* key, Bool lock)
{
    XkbDescPtr xkb = XkbGetKeyboard(display, XkbAllComponentsMask, XkbUseCoreKbd);
    if(xkb == NULL)
    {
        fprintf(stderr, "X11/XKBlib cannot get keyboard.\n");
        return False;
    }

    unsigned int mask;
    if(get_key_mask(xkb, key, &mask) == False)
    {
        fprintf(stderr, "X11/XKBlib cannot find %s.\n", key);
        XkbFreeKeyboard(xkb, 0, 1);
        return False;
    }

    if(lock)
        return XkbLockModifiers(display, XkbUseCoreKbd, mask, mask);
    else
        return XkbLockModifiers(display, XkbUseCoreKbd, mask, 0);
}

static Bool get_key_lock(Display* display, const char* key)
{
    XkbDescPtr xkb = XkbGetKeyboard(display, XkbAllComponentsMask, XkbUseCoreKbd);
    if(xkb == NULL)
    {
        fprintf(stderr, "X11/XKBlib cannot get keyboard.\n");
        return False;
    }

    unsigned int mask;
    if(get_key_mask(xkb, key, &mask) == False)
    {
        fprintf(stderr, "X11/XKBlib cannot find %s.\n", key);
        XkbFreeKeyboard(xkb, 0, 1);
        return False;
    }

    XkbStateRec xkbState;
    XkbGetState(display, XkbUseCoreKbd, &xkbState);
    unsigned int state = xkbState.locked_mods & mask;
    if(state)
        return True;
    else
        return False;
}

static Bool check_args(int argc, char* argv[])
{
    const char* keys[] = {"NumLock", "CapsLock", "ScrollLock"};
    const char* ops[] = {"on", "off", "toggle", "status"};
    if(argc != 3 || !str_in_set(keys, 3, argv[1]) || !str_in_set(ops, 4, argv[2]))
    {
        printf("Usage : %s (NumLock|CapsLock|ScrollLock) (on|off|toggle|status)\n", argv[0]);
        printf("           \ton - turn on the key lock\n");
        printf("           \toff - turn off the key lock\n");
        printf("           \ttoggle - toggle the key lock\n");
        printf("           \tstatus - get the key lock status\n");
        printf("MIT License(https://github.com/hubenchang0515/)\n");
        return False;
    }

    return True;
}

static Bool str_in_set(const char* set[], int len, const char* str)
{
    for(int i = 0; i < len; i++)
    {
        if(strcmp(set[i], str) == 0)
            return True;
    }

    return False;
}