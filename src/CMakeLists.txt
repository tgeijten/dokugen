set (PROGRAM_NAME noname)

file (GLOB SOURCE_FILES "*.h" "*.cpp")

include_directories(${XO_INCLUDE_DIR})

add_executable(${PROGRAM_NAME} ${SOURCE_FILES})

source_group("" FILES ${SOURCE_FILES})

target_link_libraries(${PROGRAM_NAME} xo)

set_target_properties(${PROGRAM_NAME} PROPERTIES
	PROJECT_LABEL ${PROGRAM_NAME}
	OUTPUT_NAME ${PROGRAM_NAME}
	)

