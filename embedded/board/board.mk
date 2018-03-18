# List of all the board related files.
BOARDSRC = ${CHIBIOS}/board/board.c

# Required include directories
BOARDINC = ${CHIBIOS}/board

# Shared variables
ALLCSRC += $(BOARDSRC)
ALLINC  += $(BOARDINC)
