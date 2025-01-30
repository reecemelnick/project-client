
#include "login_form.h"
#include "network_utils.h"
#include "signup_form.h"
#include "start_menu.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    int res;

    res = display_menu();
    if(res == 1)
    {
        start_login_form();
    }
    else if(res == 2)
    {
        start_signup_form();
    }

    return 0;
}
