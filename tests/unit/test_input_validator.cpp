#include <gtest/gtest.h>
#include <sstream>
#include "input_validator.h"
#include "json.hpp"

class InputValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset stringstreams for each test
        input.clear();
        input.str("");
        output.clear();
        output.str("");
    }

    std::stringstream input;
    std::stringstream output;
};

// ────────────────────────────────────────────────────────────────────────────
// Integer Validation Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, ParseValidatedInt_ValidInput) {
    auto result = InputValidator::parseValidatedInt("42", 0, 100);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, 42);
}

TEST_F(InputValidatorTest, ParseValidatedInt_NegativeNumber) {
    auto result = InputValidator::parseValidatedInt("-10", -50, 50);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, -10);
}

TEST_F(InputValidatorTest, ParseValidatedInt_OutOfRange) {
    auto result = InputValidator::parseValidatedInt("150", 0, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::OUT_OF_RANGE);
}

TEST_F(InputValidatorTest, ParseValidatedInt_BelowRange) {
    auto result = InputValidator::parseValidatedInt("-5", 0, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::OUT_OF_RANGE);
}

TEST_F(InputValidatorTest, ParseValidatedInt_EmptyString) {
    auto result = InputValidator::parseValidatedInt("", 0, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::EMPTY_INPUT);
}

TEST_F(InputValidatorTest, ParseValidatedInt_WhitespaceOnly) {
    auto result = InputValidator::parseValidatedInt("   ", 0, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::EMPTY_INPUT);
}

TEST_F(InputValidatorTest, ParseValidatedInt_WithWhitespace) {
    auto result = InputValidator::parseValidatedInt("  42  ", 0, 100);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, 42);
}

TEST_F(InputValidatorTest, ParseValidatedInt_InvalidFormat) {
    auto result = InputValidator::parseValidatedInt("42abc", 0, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::INVALID_FORMAT);
}

TEST_F(InputValidatorTest, ParseValidatedInt_NonNumeric) {
    auto result = InputValidator::parseValidatedInt("hello", 0, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::INVALID_FORMAT);
}

TEST_F(InputValidatorTest, ParseValidatedInt_FloatAsInt) {
    auto result = InputValidator::parseValidatedInt("42.5", 0, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::INVALID_FORMAT);
}

TEST_F(InputValidatorTest, ParseValidatedInt_IntegerOverflow) {
    std::string bigNumber = "999999999999999999999";
    auto result = InputValidator::parseValidatedInt(bigNumber, 0, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::OUT_OF_RANGE);
}

TEST_F(InputValidatorTest, GetValidatedInt_FromStream) {
    input.str("25\n");
    auto result = InputValidator::getValidatedInt(input, 0, 100);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, 25);
}

// ────────────────────────────────────────────────────────────────────────────
// Double Validation Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, ParseValidatedDouble_ValidInput) {
    auto result = InputValidator::parseValidatedDouble("42.5", 0.0, 100.0);
    EXPECT_TRUE(result.isValid());
    EXPECT_DOUBLE_EQ(result.value, 42.5);
}

TEST_F(InputValidatorTest, ParseValidatedDouble_Integer) {
    auto result = InputValidator::parseValidatedDouble("42", 0.0, 100.0);
    EXPECT_TRUE(result.isValid());
    EXPECT_DOUBLE_EQ(result.value, 42.0);
}

TEST_F(InputValidatorTest, ParseValidatedDouble_Negative) {
    auto result = InputValidator::parseValidatedDouble("-10.5", -50.0, 50.0);
    EXPECT_TRUE(result.isValid());
    EXPECT_DOUBLE_EQ(result.value, -10.5);
}

TEST_F(InputValidatorTest, ParseValidatedDouble_OutOfRange) {
    auto result = InputValidator::parseValidatedDouble("150.5", 0.0, 100.0);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::OUT_OF_RANGE);
}

TEST_F(InputValidatorTest, ParseValidatedDouble_ScientificNotation) {
    auto result = InputValidator::parseValidatedDouble("1.5e2", 0.0, 200.0);
    EXPECT_TRUE(result.isValid());
    EXPECT_DOUBLE_EQ(result.value, 150.0);
}

TEST_F(InputValidatorTest, ParseValidatedDouble_InvalidFormat) {
    auto result = InputValidator::parseValidatedDouble("42.5abc", 0.0, 100.0);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::INVALID_FORMAT);
}

TEST_F(InputValidatorTest, ParseValidatedDouble_InfinityHandling) {
    auto result = InputValidator::parseValidatedDouble("inf", 0.0, 100.0);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::INVALID_FORMAT);
}

TEST_F(InputValidatorTest, GetValidatedDouble_FromStream) {
    input.str("25.75\n");
    auto result = InputValidator::getValidatedDouble(input, 0.0, 100.0);
    EXPECT_TRUE(result.isValid());
    EXPECT_DOUBLE_EQ(result.value, 25.75);
}

// ────────────────────────────────────────────────────────────────────────────
// String Validation Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, ValidateString_ValidInput) {
    auto result = InputValidator::validateString("Hello World", 5, 20, true);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, "Hello World");
}

TEST_F(InputValidatorTest, ValidateString_EmptyAllowed) {
    auto result = InputValidator::validateString("", 0, 10, true);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, "");
}

TEST_F(InputValidatorTest, ValidateString_EmptyNotAllowed) {
    auto result = InputValidator::validateString("", 1, 10, false);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::EMPTY_INPUT);
}

TEST_F(InputValidatorTest, ValidateString_TooShort) {
    auto result = InputValidator::validateString("Hi", 5, 20, true);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::OUT_OF_RANGE);
}

TEST_F(InputValidatorTest, ValidateString_TooLong) {
    std::string longString(25, 'x');
    auto result = InputValidator::validateString(longString, 0, 20, true);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::OUT_OF_RANGE);
}

TEST_F(InputValidatorTest, ValidateString_ExactLength) {
    auto result = InputValidator::validateString("12345", 5, 5, true);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, "12345");
}

TEST_F(InputValidatorTest, GetValidatedString_FromStream) {
    input.str("Hello World\n");
    auto result = InputValidator::getValidatedString(input, 5, 20, true);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, "Hello World");
}

// ────────────────────────────────────────────────────────────────────────────
// String Sanitization Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, SanitizeString_SafeCharacters) {
    std::string input = "Hello World 123!";
    std::string result = InputValidator::sanitizeString(input);
    EXPECT_EQ(result, "Hello World 123!");
}

TEST_F(InputValidatorTest, SanitizeString_WithCustomAllowed) {
    std::string input = "Hello@World#123";
    std::string allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@";
    std::string result = InputValidator::sanitizeString(input, allowed);
    EXPECT_EQ(result, "Hello@World123");
}

TEST_F(InputValidatorTest, SanitizeString_EmptyInput) {
    std::string result = InputValidator::sanitizeString("");
    EXPECT_EQ(result, "");
}

// ────────────────────────────────────────────────────────────────────────────
// Format Validation Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, IsAlphanumericSafe_ValidInput) {
    EXPECT_TRUE(InputValidator::isAlphanumericSafe("Hello World 123"));
    EXPECT_TRUE(InputValidator::isAlphanumericSafe("test-string_with_underscores"));
    EXPECT_TRUE(InputValidator::isAlphanumericSafe(""));
}

TEST_F(InputValidatorTest, IsAlphanumericSafe_InvalidInput) {
    EXPECT_FALSE(InputValidator::isAlphanumericSafe("Hello@World"));
    EXPECT_FALSE(InputValidator::isAlphanumericSafe("test.string"));
    EXPECT_FALSE(InputValidator::isAlphanumericSafe("string with\nnewline"));
}

TEST_F(InputValidatorTest, IsValidPokemonName_ValidNames) {
    EXPECT_TRUE(InputValidator::isValidPokemonName("Pikachu"));
    EXPECT_TRUE(InputValidator::isValidPokemonName("Mr. Mime"));
    EXPECT_TRUE(InputValidator::isValidPokemonName("Nidoran-M"));
    EXPECT_TRUE(InputValidator::isValidPokemonName("Ho-Oh"));
    EXPECT_TRUE(InputValidator::isValidPokemonName("Farfetch'd"));
}

TEST_F(InputValidatorTest, IsValidPokemonName_InvalidNames) {
    EXPECT_FALSE(InputValidator::isValidPokemonName(""));
    EXPECT_FALSE(InputValidator::isValidPokemonName("Pokemon123"));
    EXPECT_FALSE(InputValidator::isValidPokemonName("Pokemon@Name"));
    std::string tooLongName(51, 'x');
    EXPECT_FALSE(InputValidator::isValidPokemonName(tooLongName));
}

TEST_F(InputValidatorTest, IsValidMoveName_ValidNames) {
    EXPECT_TRUE(InputValidator::isValidMoveName("Tackle"));
    EXPECT_TRUE(InputValidator::isValidMoveName("Thunder Punch"));
    EXPECT_TRUE(InputValidator::isValidMoveName("U-turn"));
    EXPECT_TRUE(InputValidator::isValidMoveName("Will-O-Wisp"));
}

TEST_F(InputValidatorTest, IsValidMoveName_InvalidNames) {
    EXPECT_FALSE(InputValidator::isValidMoveName(""));
    EXPECT_FALSE(InputValidator::isValidMoveName("Move123"));
    EXPECT_FALSE(InputValidator::isValidMoveName("Move@Name"));
    std::string tooLongName(51, 'x');
    EXPECT_FALSE(InputValidator::isValidMoveName(tooLongName));
}

TEST_F(InputValidatorTest, ValidateStringFormat_CustomValidator) {
    auto isNumericString = [](const std::string& str) {
        return std::all_of(str.begin(), str.end(), [](char c) { return std::isdigit(c); });
    };

    auto result = InputValidator::validateStringFormat("12345", isNumericString, "Must be numeric");
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, "12345");

    auto result2 = InputValidator::validateStringFormat("123abc", isNumericString, "Must be numeric");
    EXPECT_FALSE(result2.isValid());
    EXPECT_EQ(result2.error, InputValidator::ValidationError::INVALID_FORMAT);
    EXPECT_EQ(result2.errorMessage, "Must be numeric");
}

// ────────────────────────────────────────────────────────────────────────────
// JSON Validation Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, ValidateRequiredFields_AllPresent) {
    nlohmann::json jsonObj = {
        {"name", "Pikachu"},
        {"level", 50},
        {"type", "electric"}
    };
    
    std::vector<std::string> required = {"name", "level", "type"};
    auto result = InputValidator::validateRequiredFields(jsonObj, required);
    EXPECT_TRUE(result.isValid());
}

TEST_F(InputValidatorTest, ValidateRequiredFields_MissingField) {
    nlohmann::json jsonObj = {
        {"name", "Pikachu"},
        {"level", 50}
    };
    
    std::vector<std::string> required = {"name", "level", "type"};
    auto result = InputValidator::validateRequiredFields(jsonObj, required);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::FIELD_MISSING);
}

TEST_F(InputValidatorTest, GetJsonInt_ValidField) {
    nlohmann::json jsonObj = {{"level", 50}};
    
    auto result = InputValidator::getJsonInt(jsonObj, "level", 1, 100);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, 50);
}

TEST_F(InputValidatorTest, GetJsonInt_MissingField) {
    nlohmann::json jsonObj = {{"name", "Pikachu"}};
    
    auto result = InputValidator::getJsonInt(jsonObj, "level", 1, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::FIELD_MISSING);
}

TEST_F(InputValidatorTest, GetJsonInt_WithDefault) {
    nlohmann::json jsonObj = {{"name", "Pikachu"}};
    
    auto result = InputValidator::getJsonInt(jsonObj, "level", 1, 100, 50);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, 50);
}

TEST_F(InputValidatorTest, GetJsonInt_WrongType) {
    nlohmann::json jsonObj = {{"level", "fifty"}};
    
    auto result = InputValidator::getJsonInt(jsonObj, "level", 1, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::FIELD_WRONG_TYPE);
}

TEST_F(InputValidatorTest, GetJsonInt_OutOfRange) {
    nlohmann::json jsonObj = {{"level", 150}};
    
    auto result = InputValidator::getJsonInt(jsonObj, "level", 1, 100);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::OUT_OF_RANGE);
}

TEST_F(InputValidatorTest, GetJsonString_ValidField) {
    nlohmann::json jsonObj = {{"name", "Pikachu"}};
    
    auto result = InputValidator::getJsonString(jsonObj, "name", 1, 50);
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, "Pikachu");
}

TEST_F(InputValidatorTest, GetJsonString_MissingField) {
    nlohmann::json jsonObj = {{"level", 50}};
    
    auto result = InputValidator::getJsonString(jsonObj, "name", 1, 50);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::FIELD_MISSING);
}

TEST_F(InputValidatorTest, GetJsonString_WithDefault) {
    nlohmann::json jsonObj = {{"level", 50}};
    
    auto result = InputValidator::getJsonString(jsonObj, "name", 1, 50, std::string("Default"));
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, "Default");
}

TEST_F(InputValidatorTest, GetJsonString_WrongType) {
    nlohmann::json jsonObj = {{"name", 123}};
    
    auto result = InputValidator::getJsonString(jsonObj, "name", 1, 50);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::FIELD_WRONG_TYPE);
}

TEST_F(InputValidatorTest, GetJsonString_TooLong) {
    std::string longName(51, 'x');
    nlohmann::json jsonObj = {{"name", longName}};
    
    auto result = InputValidator::getJsonString(jsonObj, "name", 1, 50);
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::OUT_OF_RANGE);
}

TEST_F(InputValidatorTest, GetJsonDouble_ValidField) {
    nlohmann::json jsonObj = {{"multiplier", 1.5}};
    
    auto result = InputValidator::getJsonDouble(jsonObj, "multiplier", 0.0, 2.0);
    EXPECT_TRUE(result.isValid());
    EXPECT_DOUBLE_EQ(result.value, 1.5);
}

TEST_F(InputValidatorTest, GetJsonDouble_IntegerAsDouble) {
    nlohmann::json jsonObj = {{"multiplier", 2}};
    
    auto result = InputValidator::getJsonDouble(jsonObj, "multiplier", 0.0, 3.0);
    EXPECT_TRUE(result.isValid());
    EXPECT_DOUBLE_EQ(result.value, 2.0);
}

// ────────────────────────────────────────────────────────────────────────────
// Stream State Management Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, IsStreamReady_GoodStream) {
    input.str("test input");
    EXPECT_TRUE(InputValidator::isStreamReady(input));
}

TEST_F(InputValidatorTest, IsStreamReady_EmptyStream) {
    EXPECT_TRUE(InputValidator::isStreamReady(input));
}

TEST_F(InputValidatorTest, RecoverInputStream_GoodStream) {
    input.str("test input");
    EXPECT_TRUE(InputValidator::recoverInputStream(input));
    EXPECT_TRUE(input.good());
}

TEST_F(InputValidatorTest, RecoverInputStream_FailedStream) {
    input.str("test");
    input.setstate(std::ios::failbit);
    EXPECT_FALSE(input.good());
    
    EXPECT_TRUE(InputValidator::recoverInputStream(input));
    EXPECT_TRUE(input.good());
}

// ────────────────────────────────────────────────────────────────────────────
// Error Reporting Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, ErrorToString_AllErrors) {
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::SUCCESS), "Success");
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::INVALID_INPUT), "Invalid input");
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::OUT_OF_RANGE), "Value out of range");
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::STREAM_ERROR), "Stream error");
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::EMPTY_INPUT), "Empty input");
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::INVALID_FORMAT), "Invalid format");
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::JSON_PARSE_ERROR), "JSON parse error");
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::FIELD_MISSING), "Field missing");
    EXPECT_EQ(InputValidator::errorToString(InputValidator::ValidationError::FIELD_WRONG_TYPE), "Field wrong type");
}

TEST_F(InputValidatorTest, FormatErrorMessage_WithContext) {
    std::string result = InputValidator::formatErrorMessage(
        InputValidator::ValidationError::OUT_OF_RANGE,
        "Integer validation",
        "150"
    );
    EXPECT_EQ(result, "Value out of range (Integer validation: 150)");
}

TEST_F(InputValidatorTest, FormatErrorMessage_WithoutValue) {
    std::string result = InputValidator::formatErrorMessage(
        InputValidator::ValidationError::FIELD_MISSING,
        "JSON field"
    );
    EXPECT_EQ(result, "Field missing (JSON field)");
}

TEST_F(InputValidatorTest, FormatErrorMessage_WithoutContext) {
    std::string result = InputValidator::formatErrorMessage(
        InputValidator::ValidationError::INVALID_FORMAT
    );
    EXPECT_EQ(result, "Invalid format");
}

// ────────────────────────────────────────────────────────────────────────────
// Prompt With Retry Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, PromptWithRetry_SuccessFirstAttempt) {
    input.str("42\n");
    
    auto validator = [](std::istream& stream) {
        return InputValidator::getValidatedInt(stream, 0, 100);
    };
    
    auto result = InputValidator::promptWithRetry<int>(
        input, output, "Enter number", 3, validator
    );
    
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, 42);
}

TEST_F(InputValidatorTest, PromptWithRetry_SuccessSecondAttempt) {
    input.str("150\n42\n");
    
    auto validator = [](std::istream& stream) {
        return InputValidator::getValidatedInt(stream, 0, 100);
    };
    
    auto result = InputValidator::promptWithRetry<int>(
        input, output, "Enter number", 3, validator
    );
    
    EXPECT_TRUE(result.isValid());
    EXPECT_EQ(result.value, 42);
}

TEST_F(InputValidatorTest, PromptWithRetry_FailAfterMaxRetries) {
    input.str("150\n200\n300\n400\n");
    
    auto validator = [](std::istream& stream) {
        return InputValidator::getValidatedInt(stream, 0, 100);
    };
    
    auto result = InputValidator::promptWithRetry<int>(
        input, output, "Enter number", 2, validator
    );
    
    EXPECT_FALSE(result.isValid());
    EXPECT_EQ(result.error, InputValidator::ValidationError::INVALID_INPUT);
}

// ────────────────────────────────────────────────────────────────────────────
// Edge Case Tests
// ────────────────────────────────────────────────────────────────────────────

TEST_F(InputValidatorTest, IntegerBoundaryValues) {
    // Test minimum integer
    auto result1 = InputValidator::parseValidatedInt(
        std::to_string(std::numeric_limits<int>::min()),
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max()
    );
    EXPECT_TRUE(result1.isValid());
    EXPECT_EQ(result1.value, std::numeric_limits<int>::min());

    // Test maximum integer
    auto result2 = InputValidator::parseValidatedInt(
        std::to_string(std::numeric_limits<int>::max()),
        std::numeric_limits<int>::min(),
        std::numeric_limits<int>::max()
    );
    EXPECT_TRUE(result2.isValid());
    EXPECT_EQ(result2.value, std::numeric_limits<int>::max());
}

TEST_F(InputValidatorTest, DoubleBoundaryValues) {
    // Test very small positive number
    auto result1 = InputValidator::parseValidatedDouble("1e-100", 0.0, 1.0);
    EXPECT_TRUE(result1.isValid());
    EXPECT_GT(result1.value, 0.0);

    // Test very large number
    auto result2 = InputValidator::parseValidatedDouble("1e100", 0.0, 1e101);
    EXPECT_TRUE(result2.isValid());
    EXPECT_LT(result2.value, 1e101);
}

TEST_F(InputValidatorTest, StringLengthBoundaryValues) {
    // Test string at exact minimum length
    auto result1 = InputValidator::validateString("abc", 3, 10, true);
    EXPECT_TRUE(result1.isValid());

    // Test string at exact maximum length  
    auto result2 = InputValidator::validateString("abcdefghij", 3, 10, true);
    EXPECT_TRUE(result2.isValid());

    // Test string one character too short
    auto result3 = InputValidator::validateString("ab", 3, 10, true);
    EXPECT_FALSE(result3.isValid());

    // Test string one character too long
    auto result4 = InputValidator::validateString("abcdefghijk", 3, 10, true);
    EXPECT_FALSE(result4.isValid());
}