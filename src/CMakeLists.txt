set (PROGRAM_NAME dokugen)

file (GLOB SOURCE_FILES "*.h" "*.cpp")

include_directories(${XO_INCLUDE_DIR})

add_executable(${PROGRAM_NAME} ${SOURCE_FILES})
set_property(TARGET ${PROGRAM_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROGRAM_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

source_group("" FILES ${SOURCE_FILES})

target_link_libraries(${PROGRAM_NAME} xo)

set_target_properties(${PROGRAM_NAME} PROPERTIES
	PROJECT_LABEL ${PROGRAM_NAME}
	OUTPUT_NAME ${PROGRAM_NAME}
	)
