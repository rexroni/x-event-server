/* Configure the names of the scripts to be executed on each hook.
   By default all three events trigger the same hook.
   Each event will call the hook with the following command line args:
     - the X window ID
     - the value of _NET_WM_NAME or WM_NAME, (often the window title)
   Each hook can be set to NULL instead of a string to be disabled.
*/
#define X_START_HOOK "x-event-hook"
#define X_FOCUS_HOOK "x-event-hook"
#define X_TITLE_HOOK "x-event-hook"

void start_hook(Window w, char *title);
void title_hook(Window w, char *title);
void focus_hook(Window w, char *title);
