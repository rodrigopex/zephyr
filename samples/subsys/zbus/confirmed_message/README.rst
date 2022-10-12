.. _zbus-confirmed-message-sample:

Confirmed message sample
########################

Overview
********
This sample implements a code of how sending messages and wait for all subscribers to read that. We call confirmed message.

Building and Running
********************

This project outputs to the console.  It can be built and executed
on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/zbus/confirmed_message
   :host-os: unix
   :board: qemu_x86
   :goals: run

Sample Output
=============

.. code-block:: console

    I: Usb sample version 0.1-2!
    I: ----------------------------------
    I:  *** USB sending message sequence 0 to interested tasks (on-to-many) ***
    I: ----------------------------------
    I: critical1_sub received sequence: 0
    I: Data:
    I: 00 00 00 00 00 00 00 00 |........
    I: 00 00 00 00 00 00 00 00 |........
    I: ----------------------------------
    I: Critical 1 sent ACK!
    I: ----------------------------------
    I: critical2_sub received sequence: 0
    I: Data:
    I: 00 00 00 00 00 00 00 00 |........
    I: 00 00 00 00 00 00 00 00 |........
    I: ----------------------------------
    I: Critical 2 sent ACK!
    I: ----------------------------------
    I: critical3_sub received sequence: 0
    I: Data:
    I: 00 00 00 00 00 00 00 00 |........
    I: 00 00 00 00 00 00 00 00 |........
    I: ----------------------------------
    I: Critical 3 sent ACK!
    I: ----------------------------------
    I: critical4_sub received sequence: 0
    I: Data:
    I: 00 00 00 00 00 00 00 00 |........
    I: 00 00 00 00 00 00 00 00 |........
    I: ----------------------------------
    I: Critical 4 sent ACK!
    I: ----------------------------------
    I: critical5_sub received sequence: 0
    I: Data:
    I: 00 00 00 00 00 00 00 00 |........
    I: 00 00 00 00 00 00 00 00 |........
    I: ----------------------------------
    I: Critical 5 sent ACK!
    I: ----------------------------------
    I:  *** USB sending message sequence 1 to interested tasks (on-to-many) ***
    I: ----------------------------------
    I: critical1_sub received sequence: 1
    I: Data:
    I: 00 01 00 00 00 00 00 00 |........
    I: 00 00 00 00 00 00 00 00 |........
    I: ----------------------------------
    I: Critical 1 sent ACK!
    I: ----------------------------------
    <continues up to...>
    I: Sample finished with success. 10 confirmed messages sent and confirmed

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.
