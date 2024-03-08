#include "indicator.h"

ZBUS_CHAN_DEFINE(chan_indicator_cmd,       /* Name */
		 struct msg_indicator_cmd, /* Message type */
		 NULL,                     /* Validator */
		 NULL,                     /* User data */
		 ZBUS_OBSERVERS_EMPTY,     /* Observers list */
		 MSG_INDICATOR_CMD_INIT_DEFAULT /* Message initialization */);

ZBUS_CHAN_DEFINE(chan_indicator_rsp,       /* Name */
		 struct msg_indicator_rsp, /* Message type */
		 NULL,                     /* Validator */
		 NULL,                     /* User data */
		 ZBUS_OBSERVERS_EMPTY,     /* Observers list */
		 MSG_INDICATOR_RSP_INIT_DEFAULT /* Message initialization */);

ZBUS_CHAN_DEFINE(chan_indicator_evt,       /* Name */
		 struct msg_indicator_evt, /* Message type */
		 NULL,                     /* Validator */
		 NULL,                     /* User data */
		 ZBUS_OBSERVERS_EMPTY,     /* Observers list */
		 MSG_INDICATOR_EVT_INIT_DEFAULT /* Message initialization */);

#if defined(CONFIG_SHELL)

#include "nano_services.h"

NANO_SERVICE_SHELL_INIT();
NANO_SERVICE_RSP_DUMP(indicator, INDICATOR);
NANO_SERVICE_EVT_DUMP(indicator, INDICATOR);
NANO_SERVICE_CMD_HANDLER(indicator, INDICATOR);

#endif /* CONFIG_SHELL */
