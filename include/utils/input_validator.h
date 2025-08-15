#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <optional>
#include <functional>

/**
 * @brief Comprehensive input validation utilities for safe user input handling
 * 
 * This class provides a collection of static methods for validating and sanitizing
 * various types of user inputs, including numeric values, strings, and JSON data.
 * All validation functions are designed to be safe, const-correct, and unit testable.
 */
class InputValidator {
public:
    /**
     * @brief Error codes for input validation operations
     */
    enum class ValidationError {
        SUCCESS = 0,
        INVALID_INPUT,
        OUT_OF_RANGE,
        STREAM_ERROR,
        EMPTY_INPUT,
        INVALID_FORMAT,
        JSON_PARSE_ERROR,
        FIELD_MISSING,
        FIELD_WRONG_TYPE
    };

    /**
     * @brief Result wrapper for validation operations
     * @tparam T The type of the validated value
     */
    template<typename T>
    struct ValidationResult {
        T value;
        ValidationError error;
        std::string errorMessage;
        
        ValidationResult(const T& val) : value(val), error(ValidationError::SUCCESS) {}
        ValidationResult(ValidationError err, const std::string& msg) 
            : value{}, error(err), errorMessage(msg) {}
        
        bool isValid() const { return error == ValidationError::SUCCESS; }
        operator bool() const { return isValid(); }
    };

    // ────────────────────────────────────────────────────────────────────────
    // Numeric Input Validation
    // ────────────────────────────────────────────────────────────────────────

    /**
     * @brief Safely parse an integer from input stream with range validation
     * @param input Input stream to read from
     * @param min Minimum allowed value (inclusive)
     * @param max Maximum allowed value (inclusive)
     * @param prompt Optional prompt message to display
     * @return ValidationResult containing the parsed integer or error information
     */
    static ValidationResult<int> getValidatedInt(
        std::istream& input,
        int min = std::numeric_limits<int>::min(),
        int max = std::numeric_limits<int>::max(),
        const std::string& prompt = ""
    );

    /**
     * @brief Safely parse an integer from string with range validation
     * @param str String to parse
     * @param min Minimum allowed value (inclusive)
     * @param max Maximum allowed value (inclusive)
     * @return ValidationResult containing the parsed integer or error information
     */
    static ValidationResult<int> parseValidatedInt(
        const std::string& str,
        int min = std::numeric_limits<int>::min(),
        int max = std::numeric_limits<int>::max()
    );

    /**
     * @brief Safely parse a double from input stream with range validation
     * @param input Input stream to read from
     * @param min Minimum allowed value (inclusive)
     * @param max Maximum allowed value (inclusive)
     * @param prompt Optional prompt message to display
     * @return ValidationResult containing the parsed double or error information
     */
    static ValidationResult<double> getValidatedDouble(
        std::istream& input,
        double min = std::numeric_limits<double>::lowest(),
        double max = std::numeric_limits<double>::max(),
        const std::string& prompt = ""
    );

    /**
     * @brief Safely parse a double from string with range validation
     * @param str String to parse
     * @param min Minimum allowed value (inclusive)
     * @param max Maximum allowed value (inclusive)
     * @return ValidationResult containing the parsed double or error information
     */
    static ValidationResult<double> parseValidatedDouble(
        const std::string& str,
        double min = std::numeric_limits<double>::lowest(),
        double max = std::numeric_limits<double>::max()
    );

    // ────────────────────────────────────────────────────────────────────────
    // String Validation and Sanitization
    // ────────────────────────────────────────────────────────────────────────

    /**
     * @brief Get a validated string from input stream
     * @param input Input stream to read from
     * @param minLength Minimum required length
     * @param maxLength Maximum allowed length
     * @param allowEmpty Whether empty strings are allowed
     * @param prompt Optional prompt message to display
     * @return ValidationResult containing the validated string or error information
     */
    static ValidationResult<std::string> getValidatedString(
        std::istream& input,
        std::size_t minLength = 0,
        std::size_t maxLength = std::numeric_limits<std::size_t>::max(),
        bool allowEmpty = true,
        const std::string& prompt = ""
    );

    /**
     * @brief Validate a string against length and content requirements
     * @param str String to validate
     * @param minLength Minimum required length
     * @param maxLength Maximum allowed length
     * @param allowEmpty Whether empty strings are allowed
     * @return ValidationResult containing the validated string or error information
     */
    static ValidationResult<std::string> validateString(
        const std::string& str,
        std::size_t minLength = 0,
        std::size_t maxLength = std::numeric_limits<std::size_t>::max(),
        bool allowEmpty = true
    );

    /**
     * @brief Sanitize a string by removing or replacing dangerous characters
     * @param str String to sanitize
     * @param allowedChars Optional set of allowed characters (if empty, uses default safe set)
     * @return Sanitized string with potentially dangerous characters removed
     */
    static std::string sanitizeString(
        const std::string& str,
        const std::string& allowedChars = ""
    );

    /**
     * @brief Validate string format using a custom predicate function
     * @param str String to validate
     * @param validator Predicate function that returns true if string is valid
     * @param errorMessage Custom error message for validation failure
     * @return ValidationResult containing the validated string or error information
     */
    static ValidationResult<std::string> validateStringFormat(
        const std::string& str,
        std::function<bool(const std::string&)> validator,
        const std::string& errorMessage = "Invalid string format"
    );

    /**
     * @brief Check if string contains only alphanumeric characters and common safe symbols
     * @param str String to check
     * @return True if string is safe, false otherwise
     */
    static bool isAlphanumericSafe(const std::string& str);

    /**
     * @brief Check if string is a valid Pokemon name format
     * @param name Pokemon name to validate
     * @return True if name format is valid, false otherwise
     */
    static bool isValidPokemonName(const std::string& name);

    /**
     * @brief Check if string is a valid move name format
     * @param name Move name to validate
     * @return True if name format is valid, false otherwise
     */
    static bool isValidMoveName(const std::string& name);

    // ────────────────────────────────────────────────────────────────────────
    // File Path Security and Validation
    // ────────────────────────────────────────────────────────────────────────

    /**
     * @brief Validate and construct a secure file path within the data directory
     * @param fileName Base filename (without directory or extension)
     * @param dataType Type of data (pokemon, moves, teams)
     * @param requiredExtension Required file extension (e.g., ".json")
     * @return ValidationResult containing the secure path or error information
     */
    static ValidationResult<std::string> validateDataFilePath(
        const std::string& fileName,
        const std::string& dataType,
        const std::string& requiredExtension = ".json"
    );

    /**
     * @brief Check if a filename is safe and doesn't contain path traversal characters
     * @param fileName Filename to validate
     * @return True if filename is safe, false otherwise
     */
    static bool isSecureFileName(const std::string& fileName);

    /**
     * @brief Sanitize a filename by removing dangerous characters and path components
     * @param fileName Filename to sanitize
     * @return Sanitized filename safe for file operations
     */
    static std::string sanitizeFileName(const std::string& fileName);

    /**
     * @brief Validate that a file path is within the allowed data directory bounds
     * @param filePath File path to validate
     * @param allowedDataTypes Set of allowed data type directories
     * @return ValidationResult indicating whether path is safe
     */
    static ValidationResult<bool> validatePathWithinDataDirectory(
        const std::string& filePath,
        const std::vector<std::string>& allowedDataTypes = {"pokemon", "moves", "teams"}
    );

    /**
     * @brief Get the absolute canonical path and verify it's within data directory
     * @param filePath Path to canonicalize and validate
     * @return ValidationResult containing the canonical path or error
     */
    static ValidationResult<std::string> getCanonicalDataPath(const std::string& filePath);

    /**
     * @brief Validate file size and permissions before loading
     * @param filePath Path to file to validate
     * @param maxSizeBytes Maximum allowed file size in bytes
     * @return ValidationResult indicating whether file is safe to load
     */
    static ValidationResult<bool> validateFileAccessibility(
        const std::string& filePath,
        std::size_t maxSizeBytes = 1024 * 1024  // 1MB default limit
    );

    // ────────────────────────────────────────────────────────────────────────
    // JSON Field Validation Helpers
    // ────────────────────────────────────────────────────────────────────────

    /**
     * @brief Validate that a JSON object contains required fields
     * @param jsonObj JSON object to validate (using nlohmann::json)
     * @param requiredFields Vector of required field names
     * @return ValidationResult with success or field validation error
     */
    template<typename JsonType>
    static ValidationResult<bool> validateRequiredFields(
        const JsonType& jsonObj,
        const std::vector<std::string>& requiredFields
    );

    /**
     * @brief Safely extract an integer field from JSON with validation
     * @param jsonObj JSON object to extract from
     * @param fieldName Name of the field to extract
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @param defaultValue Default value if field is missing (optional)
     * @return ValidationResult containing the extracted integer or error
     */
    template<typename JsonType>
    static ValidationResult<int> getJsonInt(
        const JsonType& jsonObj,
        const std::string& fieldName,
        int min = std::numeric_limits<int>::min(),
        int max = std::numeric_limits<int>::max(),
        const std::optional<int>& defaultValue = std::nullopt
    );

    /**
     * @brief Safely extract a string field from JSON with validation
     * @param jsonObj JSON object to extract from
     * @param fieldName Name of the field to extract
     * @param minLength Minimum required length
     * @param maxLength Maximum allowed length
     * @param defaultValue Default value if field is missing (optional)
     * @return ValidationResult containing the extracted string or error
     */
    template<typename JsonType>
    static ValidationResult<std::string> getJsonString(
        const JsonType& jsonObj,
        const std::string& fieldName,
        std::size_t minLength = 0,
        std::size_t maxLength = std::numeric_limits<std::size_t>::max(),
        const std::optional<std::string>& defaultValue = std::nullopt
    );

    /**
     * @brief Safely extract a double field from JSON with validation
     * @param jsonObj JSON object to extract from
     * @param fieldName Name of the field to extract
     * @param min Minimum allowed value
     * @param max Maximum allowed value
     * @param defaultValue Default value if field is missing (optional)
     * @return ValidationResult containing the extracted double or error
     */
    template<typename JsonType>
    static ValidationResult<double> getJsonDouble(
        const JsonType& jsonObj,
        const std::string& fieldName,
        double min = std::numeric_limits<double>::lowest(),
        double max = std::numeric_limits<double>::max(),
        const std::optional<double>& defaultValue = std::nullopt
    );

    // ────────────────────────────────────────────────────────────────────────
    // Stream State Management
    // ────────────────────────────────────────────────────────────────────────

    /**
     * @brief Recover input stream from error state and clear input buffer
     * @param input Input stream to recover
     * @return True if recovery was successful, false otherwise
     */
    static bool recoverInputStream(std::istream& input);

    /**
     * @brief Clear the input buffer until newline or EOF
     * @param input Input stream to clear
     */
    static void clearInputBuffer(std::istream& input);

    /**
     * @brief Check if input stream is in a good state for reading
     * @param input Input stream to check
     * @return True if stream is ready for input, false otherwise
     */
    static bool isStreamReady(const std::istream& input);

    // ────────────────────────────────────────────────────────────────────────
    // Error Reporting and Utility Functions
    // ────────────────────────────────────────────────────────────────────────

    /**
     * @brief Convert ValidationError enum to human-readable string
     * @param error Error code to convert
     * @return String description of the error
     */
    static std::string errorToString(ValidationError error);

    /**
     * @brief Create a formatted error message with context
     * @param error Error code
     * @param context Additional context information
     * @param value Optional value that caused the error
     * @return Formatted error message string
     */
    static std::string formatErrorMessage(
        ValidationError error,
        const std::string& context = "",
        const std::string& value = ""
    );

    /**
     * @brief Prompt user for input with retry mechanism
     * @param input Input stream to read from
     * @param output Output stream for prompts and errors
     * @param prompt Message to display to user
     * @param maxRetries Maximum number of retry attempts
     * @param validator Function to validate input
     * @return ValidationResult with the validated input or final error
     */
    template<typename T>
    static ValidationResult<T> promptWithRetry(
        std::istream& input,
        std::ostream& output,
        const std::string& prompt,
        int maxRetries,
        std::function<ValidationResult<T>(std::istream&)> validator
    );

private:
    // Private helper functions for internal use
    
    /**
     * @brief Check if character is safe for general use
     * @param c Character to check
     * @return True if character is safe, false otherwise
     */
    static bool isSafeCharacter(char c);

    /**
     * @brief Get default set of allowed characters for string sanitization
     * @return String containing default safe characters
     */
    static std::string getDefaultAllowedChars();
};

// Template implementations need to be in header file for proper linking
#include "input_validator_templates.hpp"