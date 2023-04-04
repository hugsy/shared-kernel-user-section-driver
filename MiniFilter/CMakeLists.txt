message(STATUS "Configuring MinifilterDriver")

wdk_add_driver(MinifilterDriver
    KMDF
    1.15

    MinifilterDriver.cpp
)

target_compile_definitions(MinifilterDriver PRIVATE DEBUG=1)
target_link_options(MinifilterDriver PUBLIC /integritycheck)
target_link_libraries(MinifilterDriver DriverLib WDK::FLTMGR WDK::KSECDD)

set(RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/MinifilterDriver.rc)
set(INF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/MinifilterDriver.inf)
set(CERT_CN "BlahCat Corp")
set(CERT_STORE "PrivateCertStore")
set(CERT_FILE "$<TARGET_FILE_DIR:MinifilterDriver>/BlahCatTest.cer")

add_custom_command(
    TARGET MinifilterDriver POST_BUILD
    COMMAND
    powershell -ep bypass "Get-ChildItem -Recurse Cert:\\CurrentUser | Where-Object { $_.Subject -match '${CERT_CN}' } | Remove-Item"
    COMMAND
    makecert.exe -r -pe -ss ${CERT_STORE} -n CN="${CERT_CN}" -eku 1.3.6.1.5.5.7.3.3 "${CERT_FILE}"
    COMMAND
    signtool.exe sign /v /a /fd SHA256 /s ${CERT_STORE} /n "${CERT_CN}" "$<TARGET_FILE:MinifilterDriver>"
    COMMAND
    certmgr.exe -del -c -n "${CERT_CN}" -s -r currentUser ${CERT_STORE}
    COMMAND
    ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/MinifilterDriver.inf
    "$<TARGET_FILE_DIR:MinifilterDriver>/MinifilterDriver.inf"
    COMMENT
    "Signing driver with self-signed certificate"
)

install(TARGETS MinifilterDriver DESTINATION Drivers)
install(FILES $<TARGET_PDB_FILE:MinifilterDriver> ${RC_FILE} ${INF_FILE} DESTINATION Drivers/Debug)
