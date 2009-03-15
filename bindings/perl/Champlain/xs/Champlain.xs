#include "champlain-perl.h"


MODULE = Champlain  PACKAGE = Champlain  PREFIX = champlain_

PROTOTYPES: DISABLE


BOOT:
#include "register.xsh"
#include "boot.xsh"
