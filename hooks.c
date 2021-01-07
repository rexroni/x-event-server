#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <X11/Xlib.h>

#include "hooks.h"

static void call_script(Window w, char *title, char *script){
    pid_t child_pid = fork();
    if(child_pid < 0){
        perror("fork");
        return;
    }

    // are we in the child?
    if(child_pid == 0){
        char window_arg[128];
        sprintf(window_arg, "%lu", w);

        execlp(script, script, window_arg, title, NULL);

        perror(script);
        exit(127);
    }

    // just wait for the child to exit

    int wstatus;
    pid_t wret;

    // keep calling waitpid until we get a non EINTR result
    do {
        wret = waitpid(child_pid, &wstatus, 0);
    } while(wret == -1 && errno == EINTR);

    if(wret == -1){
        perror("waitpid");
        // there's no way to recover, we just leave the child dangling.
        return;
    }

    /* we actually don't care how the script exited for our own execution,
       but we print it out to be helpful */
    if(WIFEXITED(wstatus)){
        // normal exit
        if(WEXITSTATUS(wstatus) != 0){
            // failure exit code
            fprintf(
                stderr,
                "`%s %lu \"%s\"` exited %d\n",
                script, w, title, WEXITSTATUS(wstatus)
            );
        }
    }else if(WIFSIGNALED(wstatus)){
        // signal-terminated
        fprintf(
            stderr,
            "`%s %lu \"%s\"` was terminated by signal %d\n",
            script, w, title, WTERMSIG(wstatus)
        );
    }
}

void start_hook(Window w, char *title){
    // printf("start: %lu \"%s\"\n", focused, title);
    if(X_START_HOOK){
        call_script(w, title, X_START_HOOK);
    }
}

void title_hook(Window w, char *title){
    // printf("title: %lu \"%s\"\n", focused, title);
    if(X_TITLE_HOOK){
        call_script(w, title, X_TITLE_HOOK);
    }
}

void focus_hook(Window w, char *title){
    // printf("focus: %lu \"%s\"\n", focused, title);
    if(X_FOCUS_HOOK){
        call_script(w, title, X_FOCUS_HOOK);
    }
}
