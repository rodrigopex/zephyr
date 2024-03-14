#include <zephyr/shell/shell.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <zephyr/shell/shell_backend.h>
#include <zephyr/sys/util.h>

#define NANO_SERVICE_SHELL_INIT()                                                                  \
	static const struct shell *sh_uart;                                                        \
	static int init_shell()                                                                    \
	{                                                                                          \
		sh_uart = shell_backend_get_by_name("shell_uart");                                 \
		return 0;                                                                          \
	}                                                                                          \
	SYS_INIT(init_shell, APPLICATION, 99)

#define NANO_SERVICE_RSP_DUMP(service, SERVICE)                                                    \
	static void dump_listener_callback_##service##_rsp(const struct zbus_channel *chan)        \
	{                                                                                          \
		uint8_t buffer[MSG_##SERVICE##_RSP_SIZE];                                          \
		pb_ostream_t stream = pb_ostream_from_buffer(buffer, MSG_##SERVICE##_RSP_SIZE);    \
		const struct msg_##service##_rsp *serv = zbus_chan_const_msg(chan);                \
		pb_encode(&stream, MSG_##SERVICE##_RSP_FIELDS, serv);                              \
		char data_string[MSG_##SERVICE##_RSP_SIZE * 2 + 1] = {0};                          \
		bin2hex(buffer, stream.bytes_written, data_string,                                 \
			MSG_##SERVICE##_RSP_SIZE * 2 + 1);                                         \
		shell_print(sh_uart, "@%s %s", zbus_chan_name(chan), data_string);                 \
	}                                                                                          \
	ZBUS_LISTENER_DEFINE(lis_dump_##service##_rsp, dump_listener_callback_##service##_rsp);    \
	ZBUS_CHAN_ADD_OBS(chan_##service##_rsp, lis_dump_##service##_rsp, 3)

#define NANO_SERVICE_EVT_DUMP(service, SERVICE)                                                    \
	static void dump_listener_callback_##service##_evt(const struct zbus_channel *chan)        \
	{                                                                                          \
		uint8_t buffer[MSG_##SERVICE##_EVT_SIZE];                                          \
		pb_ostream_t stream = pb_ostream_from_buffer(buffer, MSG_##SERVICE##_EVT_SIZE);    \
		const struct msg_##service##_evt *serv = zbus_chan_const_msg(chan);                \
		pb_encode(&stream, MSG_##SERVICE##_EVT_FIELDS, serv);                              \
		char data_string[MSG_##SERVICE##_EVT_SIZE * 2 + 1] = {0};                          \
		bin2hex(buffer, stream.bytes_written, data_string,                                 \
			MSG_##SERVICE##_EVT_SIZE * 2 + 1);                                         \
		printk("@%s %s\n", zbus_chan_name(chan), data_string);                             \
	}                                                                                          \
	ZBUS_LISTENER_DEFINE(lis_dump_##service##_evt, dump_listener_callback_##service##_evt);    \
	ZBUS_CHAN_ADD_OBS(chan_##service##_evt, lis_dump_##service##_evt, 3)

#define NANO_SERVICE_CMD_HANDLER(service, SERVICE)                                                 \
	static int cmd_handler_##service##_cmd(const struct shell *sh, size_t argc, char **argv,   \
					       void *data)                                         \
	{                                                                                          \
		const char *hex_string = argv[1];                                                  \
		uint8_t buffer[128] = {0};                                                         \
		size_t binary_length = hex2bin(hex_string, strlen(hex_string), buffer, 128);       \
		pb_istream_t stream = pb_istream_from_buffer(buffer, binary_length);               \
		struct msg_##service##_cmd cmd = MSG_##SERVICE##_CMD_INIT_DEFAULT;                 \
		bool status = pb_decode(&stream, MSG_##SERVICE##_CMD_FIELDS, &cmd);                \
		if (status) {                                                                      \
			int err = zbus_chan_pub(&chan_##service##_cmd, &cmd, K_MSEC(500));         \
			if (err) {                                                                 \
				shell_error(sh, "!Error: could not publish the encoded data");     \
				return err;                                                        \
			}                                                                          \
		} else {                                                                           \
			shell_error(sh, "!Error: invalid encoded data");                           \
		}                                                                                  \
		return status == true;                                                             \
	}                                                                                          \
	SHELL_CMD_ARG_REGISTER(service##_cmd, NULL, #service " command interface.",                \
			       cmd_handler_##service##_cmd, 2, 0)
