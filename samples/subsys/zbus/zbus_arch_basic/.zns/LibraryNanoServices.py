#!/usr/bin/env python3

"""
Run the command `python -m robot.libdoc LibraryNanoServices LibraryNanoServices.libspec` to make the
library available on the robotframework_lsp.
"""

import google.protobuf.text_format as text_format
import serial


class LibraryNanoServices(object):
    def __init__(self, services=None, services_proto_path="", port="/dev/ttyACM0", baudrate=115200, **kwargs):
        self.__generate_python_protobuf_file(services, services_proto_path)
        if 'timeout' in kwargs.keys():
            kwargs['timeout'] = int(kwargs['timeout'])
        self.__serial = serial.Serial(port, baudrate, **kwargs)

    def __generate_python_protobuf_file(self, services, services_proto_path):
        """
        Generates the python files used by this script to manipulate the proto messages

        :param services list of string: list with the services names to be used. The name must match
        with the proto file. For example, indicator must have a indicator.proto file inside the
        proto path folder.
        :param services_proto_path the path to the proto files folder: All the proto files must be
        in the same folder for now.
        """
        if services is None:
            return

        import os
        import subprocess
        for service_name in services:
            service_py_file_path = os.path.join(
                "build", services_proto_path, f"{service_name}_pb2.py")
            service_proto_file_path = os.path.join(
                services_proto_path, f"{service_name}.proto")
            if not os.path.exists(service_py_file_path):
                subprocess.run(f"protoc -I{os.environ['ZEPHYR_BASE']}/../modules/lib/nanopb/generator/proto -I. --python_out=build {service_proto_file_path}",
                               shell=True, check=True).returncode
        import sys
        sys.path.append(
            f"{os.environ['ZEPHYR_BASE']}/../modules/lib/nanopb/generator/proto")

    def cleanup(self):
        self.__serial.close()

    def service_execute_command(self, service_name: str, **kwargs):
        from importlib import util

        spec = util.spec_from_file_location(
            f"{service_name}_pb2", f"./build/nano_services/{service_name}_pb2.py")

        if spec is None:
            raise AssertionError(
                f"Python protobuf message file {service_name}_pb2.py does not exist.")

        service = util.module_from_spec(spec)

        if service is None or spec.loader is None:
            raise AssertionError(
                f"Could not import {service_name}_pb2.py file.")

        spec.loader.exec_module(service)

        cmd_message_class = getattr(service, f"Msg{service_name.title()}Cmd")
        rsp_message_class = getattr(service, f"Msg{service_name.title()}Rsp")

        print(kwargs)

        encoded_message = text_format.Parse(
            kwargs['Cmd'], cmd_message_class()).SerializeToString().hex()
        print(f"Encoded cmd data: {encoded_message}")
        self.__serial.write(f"{service_name}_cmd {encoded_message}\n".encode())

        data = self.__serial.read_until(expected=b'uart:~$')
        data = self.__serial.read_until(expected=b'uart:~$')
        data = data.rstrip().split()[1].decode("ascii")

        rsp_msg = rsp_message_class()
        rsp_msg.ParseFromString(bytes.fromhex(data))

        expected_rsp_msg = text_format.Parse(
            kwargs['Rsp'], rsp_message_class())

        print(
            f"Cmd in text_format:\n{repr(rsp_msg)}Encoded rsp data: {data}\nRsp in text format:\n{repr(expected_rsp_msg)}")
        if rsp_msg != expected_rsp_msg:
            raise AssertionError(
                f"The result:\n{repr(rsp_msg)}\nis different from the expected:\n\n{repr(expected_rsp_msg)}")
        return True

    def service_wait_for_event(self, service_name: str, **kwargs):
        from importlib import util

        spec = util.spec_from_file_location(
            f"{service_name}_pb2", f"./build/nano_services/{service_name}_pb2.py")

        if spec is None:
            raise AssertionError(
                f"Python protobuf message file {service_name}_pb2.py does not exist.")

        service = util.module_from_spec(spec)

        if service is None or spec.loader is None:
            raise AssertionError(
                f"Could not import {service_name}_pb2.py file.")

        spec.loader.exec_module(service)

        evt_message_class = getattr(service, f"Msg{service_name.title()}Evt")

        print(kwargs)

        data = self.__serial.read_until(expected=b'@')
        data = self.__serial.read_until(expected=b'uart:~$')

        if data == b'':
            raise AssertionError(
                f"Timeout! No event from {service_name.title()} service received.")

        data = data.rstrip().split()[1].decode("ascii")

        evt_msg = evt_message_class()
        evt_msg.ParseFromString(bytes.fromhex(data))

        expected_evt_msg = text_format.Parse(
            kwargs['Evt'], evt_message_class())

        print(
            f"Evt in text_format:\n{repr(evt_msg)}Encoded evt data: {data}\nEvt in text format:\n{repr(expected_evt_msg)}")
        if evt_msg != expected_evt_msg:
            raise AssertionError(
                f"The result:\n{repr(evt_msg)}\nis different from the expected:\n\n{repr(expected_evt_msg)}")
        return True
    #
    # def service_execute_indicator_command(self, **kwargs):
    #     print(kwargs)
    #     m = getattr(indicator,"MsgIndicatorCmd")
    #     encoded_message = text_format.Parse(kwargs['Cmd'], indicator.MsgIndicatorCmd()).SerializeToString().hex()
    #     print(f"Encoded cmd data: {encoded_message}")
    #     self.__serial.write(f"indicator_cmd {encoded_message}\n".encode())
    #
    #     data =  self.__serial.read_until(expected=b'uart:~$')
    #     data =  self.__serial.read_until(expected=b'uart:~$')
    #     data = data.rstrip().split()[1].decode("ascii")
    #
    #     rsp_msg = indicator.MsgIndicatorRsp()
    #     rsp_msg.ParseFromString(bytes.fromhex(data))
    #
    #     expected_rsp_msg = text_format.Parse(kwargs['Rsp'], indicator.MsgIndicatorRsp())
    #
    #     print(f"Cmd in text_format:\n{repr(rsp_msg)}Encoded rsp data: {data}\nRsp in text format:\n{repr(expected_rsp_msg)}")
    #     if rsp_msg != expected_rsp_msg:
    #         raise AssertionError(f"The result:\n{repr(rsp_msg)}\nis different from the expected:\n\n{repr(expected_rsp_msg)}")
    #     return True
    #


# if __name__ == "__main__":
#     zb_sh = LibraryNanoServices("/dev/ttyACM0", 115200, timeout=30)
#     try:
#         print(zb_sh.service_execute_indicator_command(Cmd="get_pulse_config {}",
#                                                       Rsp="pulse_config { duration:100 }"))
#
#     except KeyboardInterrupt:
#         zb_sh.cleanup()

    # print(z.encode_protobuf_text_format_into_bytes("on {}"))
    # print(z.decode_bytes_into_protobuf_text_format("0a00"))
    # print(z.encode_protobuf_text_format_into_bytes("off {}"))
    # print(z.decode_bytes_into_protobuf_text_format("1200"))
    # print(z.encode_protobuf_text_format_into_bytes("toggle {}"))
    # print(z.decode_bytes_into_protobuf_text_format("1a00"))
    # print(z.encode_protobuf_text_format_into_bytes("pulse {}"))
    # print(z.decode_bytes_into_protobuf_text_format("2200"))
    # print(z.encode_protobuf_text_format_into_bytes("set_pulse_config {duration: 100}"))
    # print(z.decode_bytes_into_protobuf_text_format("32020864"))
    # print(z.encode_protobuf_text_format_into_bytes("set_pulse_config {duration: 1000}"))
    # print(z.decode_bytes_into_protobuf_text_format("320308e807"))
    # print(z.encode_protobuf_text_format_into_bytes("get_pulse_config {}"))
    # print(z.decode_bytes_into_protobuf_text_format("2a00"))
