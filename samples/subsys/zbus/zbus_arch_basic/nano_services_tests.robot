*** Settings ***
Library             Process
Library             Dialogs
Library             LibraryNanoServices.py    ${services}    ${services_proto_path}    ${serial_port}    ${serial_bauderate}    timeout=${serial_timeout}

Suite Teardown      Terminate All Processes    kill=True


*** Variables ***
${board}                    efr32bg22_brd4184b
${rebuild_and_flash}        True
# Service list
@{services}                 indicator    trigger
${services_proto_path}      include/services/
# Serial configuration
${serial_port}              /dev/ttyACM0
${serial_bauderate}         115200
${serial_timeout}           30


*** Test Cases ***
Indicator On Off Toggle Tests
    [Documentation]    The basic tests of indicator interactions

    IF    ${rebuild_and_flash} == True
        Log    Rebuilding code and flashing it.

        ${result}    Run Process
        ...    west build -p -b ${board} -- -DCONFIG_BUTTON_TRIGGER\=y -DCONFIG_SHELL\=y && west flash
        ...    shell=True
        Should Be Equal As Integers    ${result.rc}    0
    ELSE
        ${result}    Run Process    west flash --reset    shell=True
        Should Be Equal As Integers    ${result.rc}    0
    END

    Sleep    1

    Service Execute Command
    ...    indicator
    ...    Cmd=on {}
    ...    Rsp=state {is_on: true}

    Service Execute Command
    ...    indicator
    ...    Cmd=on {}
    ...    Rsp=state {is_on: true}

    Sleep    1

    Service Execute Command
    ...    indicator
    ...    Cmd=off {}
    ...    Rsp=state {is_on: false}

    Service Execute Command
    ...    indicator
    ...    Cmd=toggle {}
    ...    Rsp=state {is_on: true}

    Service Execute Command
    ...    indicator
    ...    Cmd=toggle {}
    ...    Rsp=state {is_on: false}

Indicator Pulse Tests
    [Documentation]    The pulse tests with get and set commands
    Service Execute Command
    ...    indicator
    ...    Cmd=pulse {}
    ...    Rsp=state {is_on: false}

    Service Execute Command
    ...    indicator
    ...    Cmd=get_pulse_config {}
    ...    Rsp=pulse_config { duration:100 }

    Service Execute Command
    ...    indicator
    ...    Cmd=pulse {}
    ...    Rsp=state {is_on: false}

    Service Execute Command
    ...    indicator
    ...    Cmd=set_pulse_config { duration: 1000}
    ...    Rsp=pulse_config { duration:1000 }

    Service Execute Command
    ...    indicator
    ...    Cmd=pulse {}
    ...    Rsp=state {is_on: false}

Indicator Push Trigger Activated Event Tests
    Execute Manual Step    Press the BTN0 on the board. The LED must blink.

    Log    Press the BTN0 on the board again to proceed with the tests    WARN

    Service Wait For Event
    ...    trigger
    ...    Evt=activated {}
