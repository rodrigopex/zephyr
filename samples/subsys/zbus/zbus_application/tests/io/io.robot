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
    Prepare Machine
    Wait For Prompt On Uart    uart:~$
    Write Line To Uart    eval 1 + 2
    Write Line To Uart    eval 10 * 2
    Write Line To Uart    eval 56 - 16
    Write Line To Uart    eval 100 / 2
    Write Line To Uart    eval 100 / 0
