#include "simdjson.h"
#include "test_ondemand.h"

using namespace simdjson;

namespace parse_api_tests {
  using namespace std;

  const padded_string BASIC_JSON = "[1,2,3]"_padded;
  const padded_string BASIC_NDJSON = "[1,2,3]\n[4,5,6]"_padded;
  const padded_string EMPTY_NDJSON = ""_padded;

  bool parser_iterate() {
    TEST_START();
    ondemand::parser parser;
    auto doc = parser.iterate(BASIC_JSON);
    ASSERT_SUCCESS( doc.get_array() );
    return true;
  }

  bool parser_iterate_padded() {
    TEST_START();
    ondemand::parser parser;
    const char json_str[] = "12\0                              "; // 32 padding
    static_assert(sizeof(json_str) > sizeof(const char*));

    {
      auto doc = parser.iterate(json_str, strlen(json_str), sizeof(json_str));
      ASSERT_SUCCESS( doc.get_double() );
    }

    {
      const uint8_t* json = reinterpret_cast<const uint8_t*>(json_str);
      auto doc = parser.iterate(json, strlen(json_str), sizeof(json_str));
      ASSERT_SUCCESS( doc.get_double() );
    }

    {
      std::string_view json(json_str);
      auto doc = parser.iterate(json, sizeof(json_str));
      ASSERT_SUCCESS( doc.get_double() );
    }

    {
      std::string json = "12";
      json.reserve(json.length() + SIMDJSON_PADDING);
      auto doc = parser.iterate(json);
      ASSERT_SUCCESS( doc.get_double() );
    }

    TEST_SUCCEED();
  }

  bool parser_iterate_padded_string_view() {
    TEST_START();
    ondemand::parser parser;
    const char json_str[] = "12\0                                ";
    static_assert(sizeof(json_str) > sizeof(const char*));

    {
      padded_string_view json(json_str, sizeof(json_str));
      auto doc = parser.iterate(json);
      ASSERT_SUCCESS( doc.get_double() );
    }

    {
      padded_string_view json(std::string_view(json_str), sizeof(json_str));
      auto doc = parser.iterate(json);
      ASSERT_SUCCESS( doc.get_double() );
    }

    {
      auto doc = parser.iterate(padded_string_view(json_str, strlen(json_str), sizeof(json_str)));
      ASSERT_SUCCESS( doc.get_double() );
    }

    {
      std::string json = "12";
      json.reserve(json.length() + SIMDJSON_PADDING);
      auto doc = parser.iterate(padded_string_view(json));
      ASSERT_SUCCESS( doc.get_double() );
    }

    TEST_SUCCEED();
  }

  bool parser_iterate_insufficient_padding() {
    TEST_START();
    ondemand::parser parser;
    constexpr char json_str[] = "12\0                             "; // 31 padding
    static_assert(sizeof(json_str) > sizeof(const char*));

    {
      ASSERT_ERROR( parser.iterate(json_str, strlen(json_str), sizeof(json_str)), INSUFFICIENT_PADDING );
      ASSERT_ERROR( parser.iterate(json_str, strlen(json_str), strlen(json_str)), INSUFFICIENT_PADDING );
    }

    {
      std::string_view json(json_str);
      ASSERT_ERROR( parser.iterate(json, sizeof(json_str)), INSUFFICIENT_PADDING );
      ASSERT_ERROR( parser.iterate(json, strlen(json_str)), INSUFFICIENT_PADDING );
    }

    {
      std::string json = "12";
      ASSERT_ERROR( parser.iterate(json), INSUFFICIENT_PADDING );
      // It's actually kind of hard to allocate "just enough" capacity, since the string tends
      // to grow more than you tell it to.
    }

    TEST_SUCCEED();
  }

#if SIMDJSON_EXCEPTIONS
  bool parser_iterate_exception() {
    TEST_START();
    ondemand::parser parser;
    auto doc = parser.iterate(BASIC_JSON);
    simdjson_unused ondemand::array array = doc;
    TEST_SUCCEED();
  }
#endif // SIMDJSON_EXCEPTIONS

  bool run() {
    return parser_iterate() &&
           parser_iterate_padded() &&
           parser_iterate_padded_string_view() &&
           parser_iterate_insufficient_padding() &&
#if SIMDJSON_EXCEPTIONS
           parser_iterate_exception() &&
#endif // SIMDJSON_EXCEPTIONS
           true;
  }
}


int main(int argc, char *argv[]) {
  return test_main(argc, argv, parse_api_tests::run);
}
