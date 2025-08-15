#include "input_validator.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <set>

// ────────────────────────────────────────────────────────────────────────────
// Numeric Input Validation Implementation
// ────────────────────────────────────────────────────────────────────────────

InputValidator::ValidationResult<int> InputValidator::getValidatedInt(
    std::istream& input,
    int min,
    int max,
    const std::string& prompt
) {
    if (!prompt.empty()) {
        std::cout << prompt << ": ";
    }

    if (!isStreamReady(input)) {
        return ValidationResult<int>(ValidationError::STREAM_ERROR, "Input stream is not ready");
    }

    std::string line;
    if (!std::getline(input, line)) {
        return ValidationResult<int>(ValidationError::STREAM_ERROR, "Failed to read input");
    }

    return parseValidatedInt(line, min, max);
}

InputValidator::ValidationResult<int> InputValidator::parseValidatedInt(
    const std::string& str,
    int min,
    int max
) {
    if (str.empty()) {
        return ValidationResult<int>(ValidationError::EMPTY_INPUT, "Input string is empty");
    }

    // Trim whitespace
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    if (trimmed.empty()) {
        return ValidationResult<int>(ValidationError::EMPTY_INPUT, "Input contains only whitespace");
    }

    try {
        std::size_t pos;
        long long value = std::stoll(trimmed, &pos);

        // Check if entire string was consumed
        if (pos != trimmed.length()) {
            return ValidationResult<int>(
                ValidationError::INVALID_FORMAT,
                "Input contains non-numeric characters: '" + trimmed + "'"
            );
        }

        // Check for overflow
        if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
            return ValidationResult<int>(
                ValidationError::OUT_OF_RANGE,
                "Value " + std::to_string(value) + " exceeds integer range"
            );
        }

        int intValue = static_cast<int>(value);

        // Check user-specified range
        if (intValue < min || intValue > max) {
            return ValidationResult<int>(
                ValidationError::OUT_OF_RANGE,
                "Value " + std::to_string(intValue) + " is outside allowed range [" +
                std::to_string(min) + ", " + std::to_string(max) + "]"
            );
        }

        return ValidationResult<int>(intValue);
    }
    catch (const std::invalid_argument&) {
        return ValidationResult<int>(
            ValidationError::INVALID_FORMAT,
            "Invalid number format: '" + trimmed + "'"
        );
    }
    catch (const std::out_of_range&) {
        return ValidationResult<int>(
            ValidationError::OUT_OF_RANGE,
            "Number too large: '" + trimmed + "'"
        );
    }
}

InputValidator::ValidationResult<double> InputValidator::getValidatedDouble(
    std::istream& input,
    double min,
    double max,
    const std::string& prompt
) {
    if (!prompt.empty()) {
        std::cout << prompt << ": ";
    }

    if (!isStreamReady(input)) {
        return ValidationResult<double>(ValidationError::STREAM_ERROR, "Input stream is not ready");
    }

    std::string line;
    if (!std::getline(input, line)) {
        return ValidationResult<double>(ValidationError::STREAM_ERROR, "Failed to read input");
    }

    return parseValidatedDouble(line, min, max);
}

InputValidator::ValidationResult<double> InputValidator::parseValidatedDouble(
    const std::string& str,
    double min,
    double max
) {
    if (str.empty()) {
        return ValidationResult<double>(ValidationError::EMPTY_INPUT, "Input string is empty");
    }

    // Trim whitespace
    std::string trimmed = str;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);

    if (trimmed.empty()) {
        return ValidationResult<double>(ValidationError::EMPTY_INPUT, "Input contains only whitespace");
    }

    try {
        std::size_t pos;
        double value = std::stod(trimmed, &pos);

        // Check if entire string was consumed
        if (pos != trimmed.length()) {
            return ValidationResult<double>(
                ValidationError::INVALID_FORMAT,
                "Input contains non-numeric characters: '" + trimmed + "'"
            );
        }

        // Check for infinity and NaN
        if (!std::isfinite(value)) {
            return ValidationResult<double>(
                ValidationError::INVALID_FORMAT,
                "Value is not finite: '" + trimmed + "'"
            );
        }

        // Check user-specified range
        if (value < min || value > max) {
            return ValidationResult<double>(
                ValidationError::OUT_OF_RANGE,
                "Value " + std::to_string(value) + " is outside allowed range [" +
                std::to_string(min) + ", " + std::to_string(max) + "]"
            );
        }

        return ValidationResult<double>(value);
    }
    catch (const std::invalid_argument&) {
        return ValidationResult<double>(
            ValidationError::INVALID_FORMAT,
            "Invalid number format: '" + trimmed + "'"
        );
    }
    catch (const std::out_of_range&) {
        return ValidationResult<double>(
            ValidationError::OUT_OF_RANGE,
            "Number too large: '" + trimmed + "'"
        );
    }
}

// ────────────────────────────────────────────────────────────────────────────
// String Validation and Sanitization Implementation
// ────────────────────────────────────────────────────────────────────────────

InputValidator::ValidationResult<std::string> InputValidator::getValidatedString(
    std::istream& input,
    std::size_t minLength,
    std::size_t maxLength,
    bool allowEmpty,
    const std::string& prompt
) {
    if (!prompt.empty()) {
        std::cout << prompt << ": ";
    }

    if (!isStreamReady(input)) {
        return ValidationResult<std::string>(ValidationError::STREAM_ERROR, "Input stream is not ready");
    }

    std::string line;
    if (!std::getline(input, line)) {
        return ValidationResult<std::string>(ValidationError::STREAM_ERROR, "Failed to read input");
    }

    return validateString(line, minLength, maxLength, allowEmpty);
}

InputValidator::ValidationResult<std::string> InputValidator::validateString(
    const std::string& str,
    std::size_t minLength,
    std::size_t maxLength,
    bool allowEmpty
) {
    if (str.empty() && !allowEmpty) {
        return ValidationResult<std::string>(ValidationError::EMPTY_INPUT, "Empty string not allowed");
    }

    if (str.length() < minLength) {
        return ValidationResult<std::string>(
            ValidationError::OUT_OF_RANGE,
            "String too short: " + std::to_string(str.length()) + " < " + std::to_string(minLength)
        );
    }

    if (str.length() > maxLength) {
        return ValidationResult<std::string>(
            ValidationError::OUT_OF_RANGE,
            "String too long: " + std::to_string(str.length()) + " > " + std::to_string(maxLength)
        );
    }

    return ValidationResult<std::string>(str);
}

std::string InputValidator::sanitizeString(
    const std::string& str,
    const std::string& allowedChars
) {
    std::string allowed = allowedChars.empty() ? getDefaultAllowedChars() : allowedChars;
    std::string result;
    result.reserve(str.length());

    for (char c : str) {
        if (allowed.find(c) != std::string::npos || isSafeCharacter(c)) {
            result += c;
        }
        // Silently skip unsafe characters
    }

    return result;
}

InputValidator::ValidationResult<std::string> InputValidator::validateStringFormat(
    const std::string& str,
    std::function<bool(const std::string&)> validator,
    const std::string& errorMessage
) {
    if (validator(str)) {
        return ValidationResult<std::string>(str);
    }
    return ValidationResult<std::string>(ValidationError::INVALID_FORMAT, errorMessage);
}

bool InputValidator::isAlphanumericSafe(const std::string& str) {
    return std::all_of(str.begin(), str.end(), [](char c) {
        return std::isalnum(c) || c == '-' || c == '_' || c == ' ';
    });
}

bool InputValidator::isValidPokemonName(const std::string& name) {
    if (name.empty() || name.length() > 50) {
        return false;
    }

    // Allow letters, spaces, hyphens, and common Pokemon name characters
    return std::all_of(name.begin(), name.end(), [](char c) {
        return std::isalpha(c) || c == ' ' || c == '-' || c == '\'' || c == '.';
    });
}

bool InputValidator::isValidMoveName(const std::string& name) {
    if (name.empty() || name.length() > 50) {
        return false;
    }

    // Allow letters, spaces, hyphens, and common move name characters
    return std::all_of(name.begin(), name.end(), [](char c) {
        return std::isalpha(c) || c == ' ' || c == '-' || c == '\'' || c == '.';
    });
}

// ────────────────────────────────────────────────────────────────────────────
// Stream State Management Implementation
// ────────────────────────────────────────────────────────────────────────────

bool InputValidator::recoverInputStream(std::istream& input) {
    if (input.good()) {
        return true;
    }

    // Clear error flags
    input.clear();
    
    // Clear the input buffer only if we can do so safely
    if (input.good() && !input.eof()) {
        clearInputBuffer(input);
    }

    // Clear any remaining error flags after buffer clear
    input.clear();

    return input.good();
}

void InputValidator::clearInputBuffer(std::istream& input) {
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

bool InputValidator::isStreamReady(const std::istream& input) {
    return input.good() && !input.eof();
}

// ────────────────────────────────────────────────────────────────────────────
// Error Reporting and Utility Functions Implementation
// ────────────────────────────────────────────────────────────────────────────

std::string InputValidator::errorToString(ValidationError error) {
    switch (error) {
        case ValidationError::SUCCESS:
            return "Success";
        case ValidationError::INVALID_INPUT:
            return "Invalid input";
        case ValidationError::OUT_OF_RANGE:
            return "Value out of range";
        case ValidationError::STREAM_ERROR:
            return "Stream error";
        case ValidationError::EMPTY_INPUT:
            return "Empty input";
        case ValidationError::INVALID_FORMAT:
            return "Invalid format";
        case ValidationError::JSON_PARSE_ERROR:
            return "JSON parse error";
        case ValidationError::FIELD_MISSING:
            return "Field missing";
        case ValidationError::FIELD_WRONG_TYPE:
            return "Field wrong type";
        default:
            return "Unknown error";
    }
}

std::string InputValidator::formatErrorMessage(
    ValidationError error,
    const std::string& context,
    const std::string& value
) {
    std::string message = errorToString(error);
    
    if (!context.empty()) {
        message += " (" + context;
        if (!value.empty()) {
            message += ": " + value;
        }
        message += ")";
    }
    
    return message;
}

// ────────────────────────────────────────────────────────────────────────────
// Private Helper Functions Implementation
// ────────────────────────────────────────────────────────────────────────────

bool InputValidator::isSafeCharacter(char c) {
    // Allow alphanumeric characters, basic punctuation, and spaces
    return std::isalnum(c) || 
           c == ' ' || c == '-' || c == '_' || c == '.' || 
           c == ',' || c == ';' || c == ':' || c == '\'' || 
           c == '"' || c == '(' || c == ')' || c == '[' || 
           c == ']' || c == '{' || c == '}' || c == '!';
}

std::string InputValidator::getDefaultAllowedChars() {
    return "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 -_.,:;'\"()[]{}!";
}


// ────────────────────────────────────────────────────────────────────────────
// File Path Security and Validation Implementation
// ────────────────────────────────────────────────────────────────────────────

InputValidator::ValidationResult<std::string> InputValidator::validateDataFilePath(
    const std::string& fileName,
    const std::string& dataType,
    const std::string& requiredExtension
) {
    // Validate inputs
    if (fileName.empty()) {
        return ValidationResult<std::string>(
            ValidationError::EMPTY_INPUT, 
            "Filename cannot be empty"
        );
    }

    if (dataType.empty()) {
        return ValidationResult<std::string>(
            ValidationError::INVALID_INPUT, 
            "Data type cannot be empty"
        );
    }

    // Validate data type is allowed
    static const std::set<std::string> allowedDataTypes = {"pokemon", "moves", "teams"};
    if (allowedDataTypes.find(dataType) == allowedDataTypes.end()) {
        return ValidationResult<std::string>(
            ValidationError::INVALID_INPUT,
            "Invalid data type: " + dataType + ". Allowed types: pokemon, moves, teams"
        );
    }

    // Check if filename is secure
    if (!isSecureFileName(fileName)) {
        return ValidationResult<std::string>(
            ValidationError::INVALID_INPUT,
            "Filename contains dangerous characters or path traversal attempts: " + fileName
        );
    }

    // Sanitize the filename
    std::string sanitizedName = sanitizeFileName(fileName);
    if (sanitizedName.empty()) {
        return ValidationResult<std::string>(
            ValidationError::INVALID_INPUT,
            "Filename becomes empty after sanitization: " + fileName
        );
    }

    // Ensure filename has the required extension
    std::string finalName = sanitizedName;
    if (!requiredExtension.empty()) {
        // Check if filename ends with required extension (C++17 compatible)
        if (finalName.length() < requiredExtension.length() ||
            finalName.substr(finalName.length() - requiredExtension.length()) != requiredExtension) {
            finalName += requiredExtension;
        }
    }

    // Construct the secure path
    std::string securePath = "data/" + dataType + "/" + finalName;

    // Validate the constructed path
    auto pathValidation = validatePathWithinDataDirectory(securePath, {dataType});
    if (!pathValidation.isValid()) {
        return ValidationResult<std::string>(pathValidation.error, pathValidation.errorMessage);
    }

    return ValidationResult<std::string>(securePath);
}

bool InputValidator::isSecureFileName(const std::string& fileName) {
    if (fileName.empty()) {
        return false;
    }

    // Check for path traversal patterns
    if (fileName.find("..") != std::string::npos ||
        fileName.find("/") != std::string::npos ||
        fileName.find("\\") != std::string::npos ||
        fileName.find(":") != std::string::npos) {
        return false;
    }

    // Check for dangerous characters
    for (char c : fileName) {
        // Allow alphanumeric, hyphens, underscores, and dots
        if (!std::isalnum(c) && c != '-' && c != '_' && c != '.') {
            return false;
        }
    }

    // Filename should not start or end with dots
    if (fileName.front() == '.' || fileName.back() == '.') {
        return false;
    }

    // Check for reserved names (Windows-specific but good practice)
    static const std::set<std::string> reservedNames = {
        "CON", "PRN", "AUX", "NUL",
        "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
        "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
    };

    std::string upperFileName = fileName;
    std::transform(upperFileName.begin(), upperFileName.end(), upperFileName.begin(), ::toupper);
    
    // Remove extension for reserved name check
    size_t dotPos = upperFileName.find_last_of('.');
    if (dotPos != std::string::npos) {
        upperFileName = upperFileName.substr(0, dotPos);
    }

    if (reservedNames.find(upperFileName) != reservedNames.end()) {
        return false;
    }

    return true;
}

std::string InputValidator::sanitizeFileName(const std::string& fileName) {
    std::string sanitized;
    sanitized.reserve(fileName.length());

    for (char c : fileName) {
        // Keep only alphanumeric, hyphens, underscores, and dots
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.') {
            sanitized += c;
        }
        // Replace spaces with hyphens
        else if (c == ' ') {
            sanitized += '-';
        }
        // Skip other characters
    }

    // Remove leading and trailing dots
    while (!sanitized.empty() && sanitized.front() == '.') {
        sanitized.erase(0, 1);
    }
    while (!sanitized.empty() && sanitized.back() == '.') {
        sanitized.pop_back();
    }

    return sanitized;
}

InputValidator::ValidationResult<bool> InputValidator::validatePathWithinDataDirectory(
    const std::string& filePath,
    const std::vector<std::string>& allowedDataTypes
) {
    try {
        // Convert to absolute path
        std::filesystem::path absolutePath = std::filesystem::absolute(filePath);
        std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(absolutePath);

        // Get the data directory absolute path
        std::filesystem::path dataDir = std::filesystem::absolute("data");
        std::filesystem::path canonicalDataDir = std::filesystem::weakly_canonical(dataDir);

        // Check if the canonical path is within the data directory
        auto relativePath = std::filesystem::relative(canonicalPath, canonicalDataDir);
        
        // If relative path starts with "..", it's outside the data directory
        std::string relativePathStr = relativePath.string();
        if (relativePath.empty() || (relativePathStr.length() >= 2 && relativePathStr.substr(0, 2) == "..")) {
            return ValidationResult<bool>(
                ValidationError::INVALID_INPUT,
                "File path attempts to access files outside data directory: " + filePath
            );
        }

        // Check if the first component matches an allowed data type
        auto pathComponents = relativePath;
        if (pathComponents.begin() != pathComponents.end()) {
            std::string firstComponent = pathComponents.begin()->string();
            
            bool isAllowedType = false;
            for (const auto& allowedType : allowedDataTypes) {
                if (firstComponent == allowedType) {
                    isAllowedType = true;
                    break;
                }
            }

            if (!isAllowedType) {
                return ValidationResult<bool>(
                    ValidationError::INVALID_INPUT,
                    "File path accesses disallowed data type: " + firstComponent
                );
            }
        }

        return ValidationResult<bool>(true);
    }
    catch (const std::filesystem::filesystem_error& e) {
        return ValidationResult<bool>(
            ValidationError::INVALID_INPUT,
            "Filesystem error validating path: " + std::string(e.what())
        );
    }
}

InputValidator::ValidationResult<std::string> InputValidator::getCanonicalDataPath(
    const std::string& filePath
) {
    try {
        std::filesystem::path absolutePath = std::filesystem::absolute(filePath);
        std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(absolutePath);
        
        // Validate the canonical path is within data directory
        auto validation = validatePathWithinDataDirectory(canonicalPath.string());
        if (!validation.isValid()) {
            return ValidationResult<std::string>(validation.error, validation.errorMessage);
        }

        return ValidationResult<std::string>(canonicalPath.string());
    }
    catch (const std::filesystem::filesystem_error& e) {
        return ValidationResult<std::string>(
            ValidationError::INVALID_INPUT,
            "Failed to canonicalize path: " + std::string(e.what())
        );
    }
}

InputValidator::ValidationResult<bool> InputValidator::validateFileAccessibility(
    const std::string& filePath,
    std::size_t maxSizeBytes
) {
    try {
        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            return ValidationResult<bool>(
                ValidationError::INVALID_INPUT,
                "File does not exist: " + filePath
            );
        }

        // Check if it's a regular file
        if (!std::filesystem::is_regular_file(filePath)) {
            return ValidationResult<bool>(
                ValidationError::INVALID_INPUT,
                "Path is not a regular file: " + filePath
            );
        }

        // Check file size
        auto fileSize = std::filesystem::file_size(filePath);
        if (fileSize > maxSizeBytes) {
            return ValidationResult<bool>(
                ValidationError::OUT_OF_RANGE,
                "File size (" + std::to_string(fileSize) + " bytes) exceeds maximum allowed (" + 
                std::to_string(maxSizeBytes) + " bytes): " + filePath
            );
        }

        // Check if file is readable
        std::ifstream testFile(filePath);
        if (!testFile.is_open()) {
            return ValidationResult<bool>(
                ValidationError::STREAM_ERROR,
                "Cannot open file for reading: " + filePath
            );
        }

        return ValidationResult<bool>(true);
    }
    catch (const std::filesystem::filesystem_error& e) {
        return ValidationResult<bool>(
            ValidationError::STREAM_ERROR,
            "Filesystem error checking file accessibility: " + std::string(e.what())
        );
    }
    catch (const std::exception& e) {
        return ValidationResult<bool>(
            ValidationError::STREAM_ERROR,
            "Error checking file accessibility: " + std::string(e.what())
        );
    }
}