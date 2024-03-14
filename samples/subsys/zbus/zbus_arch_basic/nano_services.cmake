list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)

set(NANOPB_OPTIONS "--c-style")

function(nano_service_cre_register target)
  foreach(service_proto_file IN LISTS ARGN)
    zephyr_nanopb_sources(${target} ${service_proto_file})
    get_filename_component(service_name ${service_proto_file} NAME_WE)
    set(service ${service_name})
    string(TOUPPER ${service} SERVICE_UPPER)
    configure_file(service_cre.c.in include/nano_services/${service}.c @ONLY)
    configure_file(service_cre.h.in include/nano_services/${service}.h @ONLY)
    target_sources(
      ${target}
      PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include/nano_services/${service}.c)
    message(
      WARNING ${CMAKE_CURRENT_BINARY_DIR}/include/nano_services/${service}.c)
  endforeach()
  target_include_directories(${target}
                             PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
endfunction()

function(nano_service_e_register target)
  foreach(service_proto_file IN LISTS ARGN)
    zephyr_nanopb_sources(${target} ${service_proto_file})
    get_filename_component(service_name ${service_proto_file} NAME_WE)
    set(service ${service_name})
    string(TOUPPER ${service} SERVICE_UPPER)
    configure_file(service_e.c.in include/nano_services/${service}.c @ONLY)
    configure_file(service_e.h.in include/nano_services/${service}.h @ONLY)
    target_sources(
      ${target}
      PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include/nano_services/${service}.c)
    message(
      WARNING ${CMAKE_CURRENT_BINARY_DIR}/include/nano_services/${service}.c)
  endforeach()
  target_include_directories(${target}
                             PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
endfunction()
