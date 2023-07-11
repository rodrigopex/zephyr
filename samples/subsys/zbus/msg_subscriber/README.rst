.. _zbus-msg-subscriber:

Message subscriber
##################

Overview
********
This sample illustrates how to use a message subscriber in different
ways (static and dynamic) in conjunction with other types of observer.

Building and Running
********************

This project outputs to the console.  It can be built and executed
on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/zbus/msg_subscriber
   :host-os: unix
   :board: qemu_x86
   :goals: run

Sample Output
=============

.. code-block:: console

   -- west build: running target run
   [0/1] To exit from QEMU enter: 'CTRL+a, x'[QEMU] CPU: cortex-a53
   Secondary CPU core 1 (MPID:0x1) is up
   I:  ---> Publishing to acc_data_chan channel
   I: From listener -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub1 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub2 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub3 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub4 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub5 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub6 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub7 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub8 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub9 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub10 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub11 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub12 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub13 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub14 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub15 -> Acc x=1, y=10, z=100
   I: From msg subscriber bar_msg_sub16 -> Acc x=1, y=10, z=100
   I: From subscriber bar_sub1 -> Acc x=1, y=10, z=100
   I: From subscriber bar_sub2 -> Acc x=1, y=10, z=100
   I:  ---> Publishing to acc_data_chan channel
   I: From listener -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub1 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub2 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub3 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub4 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub5 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub6 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub7 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub8 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub9 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub10 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub11 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub12 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub13 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub14 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub15 -> Acc x=2, y=20, z=200
   I: From msg subscriber bar_msg_sub16 -> Acc x=2, y=20, z=200
   I: From subscriber bar_sub1 -> Acc x=2, y=20, z=200
   I: From subscriber bar_sub2 -> Acc x=2, y=20, z=200
   I:  ---> Publishing to acc_data_chan channel
   I: From listener -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub1 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub2 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub3 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub4 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub5 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub6 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub7 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub8 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub9 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub10 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub11 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub12 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub13 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub14 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub15 -> Acc x=3, y=30, z=300
   I: From msg subscriber bar_msg_sub16 -> Acc x=3, y=30, z=300
   I: From subscriber bar_sub1 -> Acc x=3, y=30, z=300
   I: From subscriber bar_sub2 -> Acc x=3, y=30, z=300
   I:  ---> Publishing to acc_data_chan channel
   <continues>

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.
