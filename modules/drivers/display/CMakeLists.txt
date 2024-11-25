# Copyright (c) 2021 The ZMK Contributors
# SPDX-License-Identifier: MIT

zephyr_library_amend()

set_source_files_properties(
        ${ZEPHYR_BASE}/drivers/display/display_gc9a01a.c
        TARGET_DIRECTORY ${lib_name}
        PROPERTIES HEADER_FILE_ONLY ON)
        
zephyr_library_sources_ifdef(CONFIG_IL0323 il0323.c)
zephyr_library_sources_ifdef(CONFIG_LPM009M360A lpm009m360a.c)
zephyr_library_sources_ifdef(CONFIG_GC9A01A display_gc9a01a.c)



