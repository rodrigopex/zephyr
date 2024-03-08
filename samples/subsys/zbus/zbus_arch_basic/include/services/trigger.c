#include "trigger.h"
#include "nano_services.h"

ZBUS_CHAN_DEFINE(chan_trigger_evt,       /* Name */
		 struct msg_trigger_evt, /* Message type */
		 NULL,                   /* Validator */
		 NULL,                   /* User data */
		 ZBUS_OBSERVERS_EMPTY,   /* Observers list */
		 MSG_TRIGGER_EVT_INIT_DEFAULT /* Message initialization */);

#if defined(CONFIG_SHELL)

#include "nano_services.h"

NANO_SERVICE_SHELL_INIT();
NANO_SERVICE_EVT_DUMP(trigger, TRIGGER);

#endif /* CONFIG_SHELL */
