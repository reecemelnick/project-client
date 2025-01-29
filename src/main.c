
#include "login_form.h"
#include "signup_form.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    start_login_form();
    start_signup_form();

    return 0;
}
