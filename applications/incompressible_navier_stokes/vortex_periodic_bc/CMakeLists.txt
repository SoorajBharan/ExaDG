TARGETNAME(TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR})

PROJECT(${TARGET_NAME})

EXADG_PICKUP_EXE(solver.cpp ${TARGET_NAME} solver)
