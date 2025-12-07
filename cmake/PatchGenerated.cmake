# Patch generated files to fix return type mismatch for grammar_lex_destroy

message(STATUS "Patching generated files in ${GENERATED_DIR}")

set(FILES_TO_PATCH
    "${GENERATED_DIR}/Parser.C"
    "${GENERATED_DIR}/grammar.y"
)

# Patch Parser.C and grammar.y for grammar_lex_destroy return type
foreach(FILE_PATH ${FILES_TO_PATCH})
    if(EXISTS "${FILE_PATH}")
        file(READ "${FILE_PATH}" CONTENT)
        string(REPLACE "extern void grammar_lex_destroy" "extern int grammar_lex_destroy" CONTENT "${CONTENT}")
        file(WRITE "${FILE_PATH}" "${CONTENT}")
        message(STATUS "Patched ${FILE_PATH}")
    else()
        message(WARNING "File not found: ${FILE_PATH}")
    endif()
endforeach()

# Patch grammar.l to add %option nounistd for Windows compatibility
set(LEXER_FILE "${GENERATED_DIR}/grammar.l")
if(EXISTS "${LEXER_FILE}")
    file(READ "${LEXER_FILE}" CONTENT)
    if(NOT CONTENT MATCHES "%option nounistd")
        string(REPLACE "%option noyywrap" "%option noyywrap nounistd" CONTENT "${CONTENT}")
        file(WRITE "${LEXER_FILE}" "${CONTENT}")
        message(STATUS "Patched ${LEXER_FILE} with %option nounistd")
    else()
        message(STATUS "${LEXER_FILE} already contains %option nounistd")
    endif()
else()
    message(WARNING "File not found: ${LEXER_FILE}")
endif()

# Patch Lexer.C to include unistd.h for Linux/macOS 
set(LEXER_C_FILE "${GENERATED_DIR}/Lexer.C")
if(EXISTS "${LEXER_C_FILE}")
    file(READ "${LEXER_C_FILE}" CONTENT)
    # We need to ensure unistd.h is included on non-Windows systems
    if(NOT CONTENT MATCHES "#include <unistd.h>")
        string(REPLACE "#include <stdio.h>" "#include <stdio.h>\n#ifndef _WIN32\n#include <unistd.h>\n#endif" CONTENT "${CONTENT}")
        file(WRITE "${LEXER_C_FILE}" "${CONTENT}")
        message(STATUS "Patched ${LEXER_C_FILE} to include unistd.h on non-Windows systems")
    endif()
else()
    # Lexer.C might not exist yet if we are patching before Flex runs
    message(STATUS "${LEXER_C_FILE} not found (yet), skipping patch")
endif()
