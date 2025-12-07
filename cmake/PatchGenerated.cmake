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
