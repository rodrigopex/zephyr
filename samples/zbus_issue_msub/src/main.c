// #include <ThriveComs.h>
#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

struct hello_msg{
    char str[32];
    int i;
};

ZBUS_CHAN_DEFINE(hello_world_chan, struct hello_msg, NULL, NULL, ZBUS_OBSERVERS_EMPTY, {});
ZBUS_MSG_SUBSCRIBER_DEFINE(hello_world_sub);
void sub_thread(){
    const struct zbus_channel *chan;
    struct hello_msg msg;

    while(1){
        zbus_sub_wait_msg(&hello_world_sub, &chan, &msg, K_FOREVER);
        printk("SUB: %s %d\n", msg.str, msg.i);

    }
}
K_THREAD_DEFINE(hello_world_sub_thread, 1024, sub_thread, NULL, NULL, NULL, 3, 0, 0);

static void hello_listener(const struct zbus_channel *chan)
{
	const struct hello_msg *msg = (struct hello_msg *)zbus_chan_const_msg(chan);
	printk("LIS: %s %d\n", msg->str, msg->i);
}
ZBUS_LISTENER_DEFINE(hello_world_lis, hello_listener);

int main(void){
    struct hello_msg msg;
    strcpy(msg.str, "hello world");
    zbus_chan_add_obs(&hello_world_chan, &hello_world_lis, K_MSEC(200));
    zbus_chan_add_obs(&hello_world_chan, &hello_world_sub, K_MSEC(200));

    int i = 0;
    while(1){
        msg.i = ++i;
        printk("\nPUB: %s %d\n", msg.str, msg.i);
        zbus_chan_pub(&hello_world_chan, &msg, K_SECONDS(1));
        k_msleep(1000);
    }
    return 0;
}
