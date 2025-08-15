#pragma once

#include <type_traits>

// Template implementations for InputValidator class

template<typename JsonType>
InputValidator::ValidationResult<bool> InputValidator::validateRequiredFields(
    const JsonType& jsonObj,
    const std::vector<std::string>& requiredFields
) {
    for (const auto& field : requiredFields) {
        if (jsonObj.find(field) == jsonObj.end()) {
            return ValidationResult<bool>(
                ValidationError::FIELD_MISSING,
                formatErrorMessage(ValidationError::FIELD_MISSING, "Required field", field)
            );
        }
    }
    return ValidationResult<bool>(true);
}

template<typename JsonType>
InputValidator::ValidationResult<int> InputValidator::getJsonInt(
    const JsonType& jsonObj,
    const std::string& fieldName,
    int min,
    int max,
    const std::optional<int>& defaultValue
) {
    auto it = jsonObj.find(fieldName);
    if (it == jsonObj.end()) {
        if (defaultValue.has_value()) {
            // Validate default value is in range
            if (defaultValue.value() < min || defaultValue.value() > max) {
                return ValidationResult<int>(
                    ValidationError::OUT_OF_RANGE,
                    formatErrorMessage(ValidationError::OUT_OF_RANGE, "Default value for field", fieldName)
                );
            }
            return ValidationResult<int>(defaultValue.value());
        }
        return ValidationResult<int>(
            ValidationError::FIELD_MISSING,
            formatErrorMessage(ValidationError::FIELD_MISSING, "JSON field", fieldName)
        );
    }

    if (!it->is_number_integer()) {
        return ValidationResult<int>(
            ValidationError::FIELD_WRONG_TYPE,
            formatErrorMessage(ValidationError::FIELD_WRONG_TYPE, "Expected integer for field", fieldName)
        );
    }

    int value = it->template get<int>();
    if (value < min || value > max) {
        return ValidationResult<int>(
            ValidationError::OUT_OF_RANGE,
            formatErrorMessage(ValidationError::OUT_OF_RANGE, 
                "Value " + std::to_string(value) + " for field", fieldName)
        );
    }

    return ValidationResult<int>(value);
}

template<typename JsonType>
InputValidator::ValidationResult<std::string> InputValidator::getJsonString(
    const JsonType& jsonObj,
    const std::string& fieldName,
    std::size_t minLength,
    std::size_t maxLength,
    const std::optional<std::string>& defaultValue
) {
    auto it = jsonObj.find(fieldName);
    if (it == jsonObj.end()) {
        if (defaultValue.has_value()) {
            // Validate default value length
            if (defaultValue.value().length() < minLength || defaultValue.value().length() > maxLength) {
                return ValidationResult<std::string>(
                    ValidationError::OUT_OF_RANGE,
                    formatErrorMessage(ValidationError::OUT_OF_RANGE, "Default value length for field", fieldName)
                );
            }
            return ValidationResult<std::string>(defaultValue.value());
        }
        return ValidationResult<std::string>(
            ValidationError::FIELD_MISSING,
            formatErrorMessage(ValidationError::FIELD_MISSING, "JSON field", fieldName)
        );
    }

    if (!it->is_string()) {
        return ValidationResult<std::string>(
            ValidationError::FIELD_WRONG_TYPE,
            formatErrorMessage(ValidationError::FIELD_WRONG_TYPE, "Expected string for field", fieldName)
        );
    }

    std::string value = it->template get<std::string>();
    if (value.length() < minLength || value.length() > maxLength) {
        return ValidationResult<std::string>(
            ValidationError::OUT_OF_RANGE,
            formatErrorMessage(ValidationError::OUT_OF_RANGE, 
                "String length " + std::to_string(value.length()) + " for field", fieldName)
        );
    }

    return ValidationResult<std::string>(value);
}

template<typename JsonType>
InputValidator::ValidationResult<double> InputValidator::getJsonDouble(
    const JsonType& jsonObj,
    const std::string& fieldName,
    double min,
    double max,
    const std::optional<double>& defaultValue
) {
    auto it = jsonObj.find(fieldName);
    if (it == jsonObj.end()) {
        if (defaultValue.has_value()) {
            // Validate default value is in range
            if (defaultValue.value() < min || defaultValue.value() > max) {
                return ValidationResult<double>(
                    ValidationError::OUT_OF_RANGE,
                    formatErrorMessage(ValidationError::OUT_OF_RANGE, "Default value for field", fieldName)
                );
            }
            return ValidationResult<double>(defaultValue.value());
        }
        return ValidationResult<double>(
            ValidationError::FIELD_MISSING,
            formatErrorMessage(ValidationError::FIELD_MISSING, "JSON field", fieldName)
        );
    }

    if (!it->is_number()) {
        return ValidationResult<double>(
            ValidationError::FIELD_WRONG_TYPE,
            formatErrorMessage(ValidationError::FIELD_WRONG_TYPE, "Expected number for field", fieldName)
        );
    }

    double value = it->template get<double>();
    if (value < min || value > max) {
        return ValidationResult<double>(
            ValidationError::OUT_OF_RANGE,
            formatErrorMessage(ValidationError::OUT_OF_RANGE, 
                "Value " + std::to_string(value) + " for field", fieldName)
        );
    }

    return ValidationResult<double>(value);
}

template<typename T>
InputValidator::ValidationResult<T> InputValidator::promptWithRetry(
    std::istream& input,
    std::ostream& output,
    const std::string& prompt,
    int maxRetries,
    std::function<ValidationResult<T>(std::istream&)> validator
) {
    ValidationResult<T> result(ValidationError::INVALID_INPUT, "No attempts made");
    
    for (int attempt = 0; attempt <= maxRetries; ++attempt) {
        if (!prompt.empty()) {
            output << prompt;
            if (attempt > 0) {
                output << " (Attempt " << (attempt + 1) << "/" << (maxRetries + 1) << ")";
            }
            output << ": ";
        }

        result = validator(input);
        
        if (result.isValid()) {
            return result;
        }

        if (attempt < maxRetries) {
            output << "Error: " << result.errorMessage << std::endl;
            recoverInputStream(input);
        }
    }

    return ValidationResult<T>(
        ValidationError::INVALID_INPUT,
        "Failed after " + std::to_string(maxRetries + 1) + " attempts. Last error: " + result.errorMessage
    );
}