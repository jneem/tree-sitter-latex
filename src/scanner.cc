#include <algorithm>
#include <vector>
#include <string>
#include <cwctype>
#include <cstring>
#include <iostream>

#include "tree_sitter/parser.h"


namespace {

using std::vector;
using std::string;

enum TokenType {
  VERB_BODY,
  VERB_DELIM
};

struct Scanner {
  char start_delim = 0;


  Scanner() {}


  unsigned serialize(char *buffer)
  {
    return 0;
  }


  void deserialize(const char *buffer, unsigned length) {}


  bool scan_start_verb_delim(TSLexer *lexer)
  {
    // NOTE: `*` is not a valid delimiter, but should be gobbled by the main grammar
    // Also NOTE: ' ' (space) is a perfectly valid delim, as is %
    switch (lexer->lookahead)
    {
      case '\n':
      case 0:
        return false;
      default:
        start_delim = lexer->lookahead;
        lexer->advance(lexer, false);
        lexer->mark_end(lexer);
        lexer->result_symbol = VERB_DELIM;
        return true;
    }
  }

  bool scan_end_verb_delim(TSLexer *lexer)
  {
    if (lexer->lookahead == start_delim)
    {
      lexer->advance(lexer, false);
      lexer->mark_end(lexer);
      lexer->result_symbol = VERB_DELIM;
      start_delim = 0;
      return true;
    }
    else if (lexer->lookahead == '\n')
    {
      lexer->mark_end(lexer);
      lexer->result_symbol = VERB_DELIM; // don't eat the newline (for consistency)
      start_delim = 0;
      return true;
    }
    else
    {
      return false;
    }
  }

  bool scan_verb_body(TSLexer *lexer)
  {
    while (lexer->lookahead)
    {
      if (lexer->lookahead == start_delim || lexer->lookahead == '\n')
      {
        break;
      }
      else
      {
        lexer->advance(lexer, false);
      }
    }

    lexer->mark_end(lexer);
    lexer->result_symbol = VERB_BODY;
    return true;
  }


  bool scan(TSLexer *lexer, const bool *valid_symbols)
  {
    if (valid_symbols[VERB_BODY])
    {
      return scan_verb_body(lexer);
    }
    else if (valid_symbols[VERB_DELIM])
    {
      if (start_delim == 0)
      {
        return scan_start_verb_delim(lexer);
      }
      else
      {
        return scan_end_verb_delim(lexer);
      }
    }

    return false;
  }

};

}


// Source for required functions:
// https://github.com/tree-sitter/tree-sitter/blob/78b54810a62580537b0a2df7eb781e03667d63ba/src/compiler/generate_code/c_code.cc#L546-L552

extern "C" {

void *tree_sitter_latex_external_scanner_create()
{
  return new Scanner();
}

bool tree_sitter_latex_external_scanner_scan(
  void *payload, TSLexer *lexer, const bool *valid_symbols
)
{
  Scanner *scanner = static_cast<Scanner *>(payload);
  return scanner->scan(lexer, valid_symbols);
}

unsigned tree_sitter_latex_external_scanner_serialize(void *payload, char *buffer)
{
  Scanner *scanner = static_cast<Scanner *>(payload);
  return scanner->serialize(buffer);
}

void tree_sitter_latex_external_scanner_deserialize(void *payload, const char *buffer, unsigned length)
{
  Scanner *scanner = static_cast<Scanner *>(payload);
  scanner->deserialize(buffer, length);
}

void tree_sitter_latex_external_scanner_destroy(void *payload)
{
  Scanner *scanner = static_cast<Scanner *>(payload);
  delete scanner;
}

}
