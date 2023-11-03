*** Comments ***
# west twister --platform m2gl025_miv -T . -v


*** Settings ***
Resource            ${RENODEKEYWORDS}
Resource            ../../../../../../tests/robot/common.robot

Suite Setup         Run Keywords
...                     Setup
Suite Teardown      Teardown
Test Setup          Reset Emulation


*** Test Cases ***
Should Read Version From Shell
    # `Prepare Machine` keyword comes from $ZEPHYR_BASE/tests/robot/common.robot file, which is imported as a resource
    Prepare Machine
    Wait For Prompt On Uart    uart:~$
    Write Line To Uart    eval 1 + 2
    Wait For Line On Uart    1 + 2 = 3
    Write Line To Uart    eval 10 * 2
    Wait For Line On Uart    10 * 2 = 20
