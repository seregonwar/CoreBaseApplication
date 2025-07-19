# Security Guidelines

This document outlines security best practices, guidelines, and implementation strategies for CoreBaseApplication (CBA). Security is a critical aspect of any application, and this guide provides comprehensive coverage of security considerations.

## Table of Contents

- [Security Principles](#security-principles)
- [Input Validation](#input-validation)
- [Authentication and Authorization](#authentication-and-authorization)
- [Cryptography](#cryptography)
- [Secure Communication](#secure-communication)
- [Memory Safety](#memory-safety)
- [Configuration Security](#configuration-security)
- [Logging and Monitoring](#logging-and-monitoring)
- [Dependency Management](#dependency-management)
- [Security Testing](#security-testing)

## Security Principles

### Defense in Depth

Implement multiple layers of security controls:

```cpp
// Example: Multi-layer validation
class SecureUserManager {
public:
    enum class ValidationResult {
        SUCCESS,
        INVALID_INPUT,
        AUTHENTICATION_FAILED,
        AUTHORIZATION_FAILED,
        RATE_LIMITED
    };
    
    ValidationResult authenticateUser(const std::string& username, 
                                    const std::string& password,
                                    const std::string& clientIP) {
        // Layer 1: Input validation
        if (!validateInput(username, password)) {
            LOG_SECURITY_WARNING("Invalid input detected from IP: " + clientIP);
            return ValidationResult::INVALID_INPUT;
        }
        
        // Layer 2: Rate limiting
        if (!checkRateLimit(clientIP)) {
            LOG_SECURITY_WARNING("Rate limit exceeded for IP: " + clientIP);
            return ValidationResult::RATE_LIMITED;
        }
        
        // Layer 3: Authentication
        if (!authenticateCredentials(username, password)) {
            LOG_SECURITY_WARNING("Authentication failed for user: " + username + " from IP: " + clientIP);
            incrementFailedAttempts(username, clientIP);
            return ValidationResult::AUTHENTICATION_FAILED;
        }
        
        // Layer 4: Authorization
        if (!isUserAuthorized(username)) {
            LOG_SECURITY_WARNING("Unauthorized access attempt by user: " + username);
            return ValidationResult::AUTHORIZATION_FAILED;
        }
        
        LOG_SECURITY_INFO("Successful authentication for user: " + username);
        resetFailedAttempts(username);
        return ValidationResult::SUCCESS;
    }
    
private:
    bool validateInput(const std::string& username, const std::string& password);
    bool checkRateLimit(const std::string& clientIP);
    bool authenticateCredentials(const std::string& username, const std::string& password);
    bool isUserAuthorized(const std::string& username);
    void incrementFailedAttempts(const std::string& username, const std::string& clientIP);
    void resetFailedAttempts(const std::string& username);
};
```

### Principle of Least Privilege

```cpp
// Role-based access control
class AccessControl {
public:
    enum class Permission {
        READ,
        WRITE,
        DELETE,
        ADMIN
    };
    
    enum class Role {
        GUEST,
        USER,
        MODERATOR,
        ADMIN
    };
    
    class User {
    public:
        explicit User(const std::string& id, Role role) : m_id(id), m_role(role) {}
        
        bool hasPermission(Permission permission) const {
            return m_permissions.count(permission) > 0;
        }
        
        Role getRole() const { return m_role; }
        const std::string& getId() const { return m_id; }
        
    private:
        std::string m_id;
        Role m_role;
        std::set<Permission> m_permissions;
        
        friend class AccessControl;
    };
    
    static void initializeRolePermissions() {
        // Guest: no permissions
        s_rolePermissions[Role::GUEST] = {};
        
        // User: read only
        s_rolePermissions[Role::USER] = {Permission::READ};
        
        // Moderator: read and write
        s_rolePermissions[Role::MODERATOR] = {Permission::READ, Permission::WRITE};
        
        // Admin: all permissions
        s_rolePermissions[Role::ADMIN] = {
            Permission::READ, Permission::WRITE, Permission::DELETE, Permission::ADMIN
        };
    }
    
    static bool checkPermission(const User& user, Permission permission) {
        auto it = s_rolePermissions.find(user.getRole());
        if (it != s_rolePermissions.end()) {
            return it->second.count(permission) > 0;
        }
        return false;
    }
    
private:
    static std::unordered_map<Role, std::set<Permission>> s_rolePermissions;
};
```

### Fail Securely

```cpp
// Secure failure handling
class SecureFileAccess {
public:
    enum class AccessResult {
        SUCCESS,
        FILE_NOT_FOUND,
        ACCESS_DENIED,
        SECURITY_VIOLATION
    };
    
    AccessResult readFile(const std::string& filename, const User& user, std::string& content) {
        try {
            // Validate file path to prevent directory traversal
            if (!isValidFilePath(filename)) {
                LOG_SECURITY_WARNING("Invalid file path attempted: " + filename + " by user: " + user.getId());
                return AccessResult::SECURITY_VIOLATION;
            }
            
            // Check user permissions
            if (!AccessControl::checkPermission(user, AccessControl::Permission::READ)) {
                LOG_SECURITY_WARNING("Unauthorized file access attempted by user: " + user.getId());
                return AccessResult::ACCESS_DENIED;
            }
            
            // Attempt to read file
            std::ifstream file(filename);
            if (!file.is_open()) {
                // Don't reveal whether file exists to unauthorized users
                if (user.getRole() == AccessControl::Role::ADMIN) {
                    return AccessResult::FILE_NOT_FOUND;
                } else {
                    return AccessResult::ACCESS_DENIED;
                }
            }
            
            content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            return AccessResult::SUCCESS;
            
        } catch (const std::exception& e) {
            LOG_SECURITY_ERROR("File access error: " + std::string(e.what()));
            // Never expose internal errors to users
            return AccessResult::ACCESS_DENIED;
        }
    }
    
private:
    bool isValidFilePath(const std::string& path) {
        // Prevent directory traversal attacks
        if (path.find("..") != std::string::npos) {
            return false;
        }
        
        // Ensure path is within allowed directories
        std::filesystem::path normalizedPath = std::filesystem::canonical(path);
        std::filesystem::path allowedBase = std::filesystem::canonical("/allowed/directory");
        
        return normalizedPath.string().substr(0, allowedBase.string().length()) == allowedBase.string();
    }
};
```

## Input Validation

### Comprehensive Input Validation

```cpp
// Input validation framework
class InputValidator {
public:
    enum class ValidationError {
        NONE,
        NULL_INPUT,
        EMPTY_INPUT,
        TOO_LONG,
        TOO_SHORT,
        INVALID_FORMAT,
        CONTAINS_MALICIOUS_CONTENT,
        ENCODING_ERROR
    };
    
    struct ValidationResult {
        ValidationError error = ValidationError::NONE;
        std::string message;
        
        bool isValid() const { return error == ValidationError::NONE; }
    };
    
    // Email validation
    static ValidationResult validateEmail(const std::string& email) {
        if (email.empty()) {
            return {ValidationError::EMPTY_INPUT, "Email cannot be empty"};
        }
        
        if (email.length() > 254) {
            return {ValidationError::TOO_LONG, "Email too long"};
        }
        
        // Basic email format validation
        std::regex emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        if (!std::regex_match(email, emailRegex)) {
            return {ValidationError::INVALID_FORMAT, "Invalid email format"};
        }
        
        // Check for malicious content
        if (containsMaliciousContent(email)) {
            return {ValidationError::CONTAINS_MALICIOUS_CONTENT, "Email contains invalid characters"};
        }
        
        return {ValidationError::NONE, ""};
    }
    
    // Password validation
    static ValidationResult validatePassword(const std::string& password) {
        if (password.empty()) {
            return {ValidationError::EMPTY_INPUT, "Password cannot be empty"};
        }
        
        if (password.length() < 8) {
            return {ValidationError::TOO_SHORT, "Password must be at least 8 characters"};
        }
        
        if (password.length() > 128) {
            return {ValidationError::TOO_LONG, "Password too long"};
        }
        
        // Check password complexity
        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        
        for (char c : password) {
            if (std::isupper(c)) hasUpper = true;
            else if (std::islower(c)) hasLower = true;
            else if (std::isdigit(c)) hasDigit = true;
            else if (std::ispunct(c)) hasSpecial = true;
        }
        
        if (!(hasUpper && hasLower && hasDigit && hasSpecial)) {
            return {ValidationError::INVALID_FORMAT, 
                   "Password must contain uppercase, lowercase, digit, and special character"};
        }
        
        return {ValidationError::NONE, ""};
    }
    
    // SQL injection prevention
    static ValidationResult validateSqlInput(const std::string& input) {
        if (input.empty()) {
            return {ValidationError::NONE, ""};
        }
        
        // Check for SQL injection patterns
        std::vector<std::string> sqlKeywords = {
            "SELECT", "INSERT", "UPDATE", "DELETE", "DROP", "CREATE", "ALTER",
            "EXEC", "EXECUTE", "UNION", "SCRIPT", "--", "/*", "*/", "xp_", "sp_"
        };
        
        std::string upperInput = input;
        std::transform(upperInput.begin(), upperInput.end(), upperInput.begin(), ::toupper);
        
        for (const auto& keyword : sqlKeywords) {
            if (upperInput.find(keyword) != std::string::npos) {
                return {ValidationError::CONTAINS_MALICIOUS_CONTENT, "Input contains SQL keywords"};
            }
        }
        
        return {ValidationError::NONE, ""};
    }
    
    // XSS prevention
    static std::string sanitizeHtml(const std::string& input) {
        std::string result = input;
        
        // Replace dangerous characters
        std::unordered_map<char, std::string> replacements = {
            {'<', "&lt;"},
            {'>', "&gt;"},
            {'&', "&amp;"},
            {'"', "&quot;"},
            {'\', "&#x27;"}
        };
        
        for (const auto& [ch, replacement] : replacements) {
            size_t pos = 0;
            while ((pos = result.find(ch, pos)) != std::string::npos) {
                result.replace(pos, 1, replacement);
                pos += replacement.length();
            }
        }
        
        return result;
    }
    
private:
    static bool containsMaliciousContent(const std::string& input) {
        // Check for null bytes
        if (input.find('\0') != std::string::npos) {
            return true;
        }
        
        // Check for control characters
        for (char c : input) {
            if (std::iscntrl(c) && c != '\t' && c != '\n' && c != '\r') {
                return true;
            }
        }
        
        return false;
    }
};
```

### Parameter Binding for SQL

```cpp
// Safe database operations
class SecureDatabase {
public:
    class PreparedStatement {
    public:
        explicit PreparedStatement(sqlite3* db, const std::string& sql) : m_db(db) {
            int result = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &m_stmt, nullptr);
            if (result != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(m_db)));
            }
        }
        
        ~PreparedStatement() {
            if (m_stmt) {
                sqlite3_finalize(m_stmt);
            }
        }
        
        void bindString(int index, const std::string& value) {
            int result = sqlite3_bind_text(m_stmt, index, value.c_str(), -1, SQLITE_STATIC);
            if (result != SQLITE_OK) {
                throw std::runtime_error("Failed to bind string parameter");
            }
        }
        
        void bindInt(int index, int value) {
            int result = sqlite3_bind_int(m_stmt, index, value);
            if (result != SQLITE_OK) {
                throw std::runtime_error("Failed to bind integer parameter");
            }
        }
        
        bool execute() {
            int result = sqlite3_step(m_stmt);
            sqlite3_reset(m_stmt);
            return result == SQLITE_DONE || result == SQLITE_ROW;
        }
        
    private:
        sqlite3* m_db;
        sqlite3_stmt* m_stmt = nullptr;
    };
    
    // Safe user lookup - prevents SQL injection
    std::optional<User> findUser(const std::string& username) {
        // Validate input first
        auto validation = InputValidator::validateSqlInput(username);
        if (!validation.isValid()) {
            LOG_SECURITY_WARNING("Invalid SQL input detected: " + validation.message);
            return std::nullopt;
        }
        
        try {
            PreparedStatement stmt(m_db, "SELECT id, username, role FROM users WHERE username = ?");
            stmt.bindString(1, username);
            
            if (stmt.execute()) {
                // Process result...
                return User{/* ... */};
            }
        } catch (const std::exception& e) {
            LOG_SECURITY_ERROR("Database error: " + std::string(e.what()));
        }
        
        return std::nullopt;
    }
    
private:
    sqlite3* m_db;
};
```

## Authentication and Authorization

### Secure Password Handling

```cpp
// Secure password management
class PasswordManager {
public:
    struct HashedPassword {
        std::string hash;
        std::string salt;
        int iterations;
    };
    
    // Generate cryptographically secure salt
    static std::string generateSalt(size_t length = 32) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::string salt;
        salt.reserve(length);
        
        for (size_t i = 0; i < length; ++i) {
            salt.push_back(static_cast<char>(dis(gen)));
        }
        
        return base64Encode(salt);
    }
    
    // Hash password using PBKDF2
    static HashedPassword hashPassword(const std::string& password, 
                                     const std::string& salt = "",
                                     int iterations = 100000) {
        std::string actualSalt = salt.empty() ? generateSalt() : salt;
        
        // Use PBKDF2 with SHA-256
        unsigned char hash[32];
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                         reinterpret_cast<const unsigned char*>(actualSalt.c_str()), actualSalt.length(),
                         iterations, EVP_sha256(), 32, hash);
        
        std::string hashStr(reinterpret_cast<char*>(hash), 32);
        
        return {
            base64Encode(hashStr),
            actualSalt,
            iterations
        };
    }
    
    // Verify password
    static bool verifyPassword(const std::string& password, const HashedPassword& stored) {
        auto computed = hashPassword(password, stored.salt, stored.iterations);
        
        // Use constant-time comparison to prevent timing attacks
        return constantTimeCompare(computed.hash, stored.hash);
    }
    
    // Check password strength
    static int calculatePasswordStrength(const std::string& password) {
        int score = 0;
        
        // Length bonus
        if (password.length() >= 8) score += 1;
        if (password.length() >= 12) score += 1;
        if (password.length() >= 16) score += 1;
        
        // Character variety
        bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (std::islower(c)) hasLower = true;
            else if (std::isupper(c)) hasUpper = true;
            else if (std::isdigit(c)) hasDigit = true;
            else if (std::ispunct(c)) hasSpecial = true;
        }
        
        if (hasLower) score += 1;
        if (hasUpper) score += 1;
        if (hasDigit) score += 1;
        if (hasSpecial) score += 1;
        
        // Check against common passwords
        if (!isCommonPassword(password)) score += 1;
        
        return score;
    }
    
private:
    static bool constantTimeCompare(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        volatile int result = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            result |= a[i] ^ b[i];
        }
        
        return result == 0;
    }
    
    static bool isCommonPassword(const std::string& password) {
        // Check against list of common passwords
        static const std::unordered_set<std::string> commonPasswords = {
            "password", "123456", "password123", "admin", "qwerty",
            "letmein", "welcome", "monkey", "1234567890"
        };
        
        return commonPasswords.count(password) > 0;
    }
    
    static std::string base64Encode(const std::string& input) {
        // Base64 encoding implementation
        // ...
        return "";
    }
};
```

### JWT Token Management

```cpp
// JWT token handling
class JWTManager {
public:
    struct TokenClaims {
        std::string userId;
        std::string role;
        std::chrono::system_clock::time_point issuedAt;
        std::chrono::system_clock::time_point expiresAt;
        std::string issuer;
    };
    
    explicit JWTManager(const std::string& secretKey) : m_secretKey(secretKey) {
        if (secretKey.length() < 32) {
            throw std::invalid_argument("JWT secret key must be at least 32 characters");
        }
    }
    
    std::string generateToken(const TokenClaims& claims) {
        nlohmann::json header = {
            {"alg", "HS256"},
            {"typ", "JWT"}
        };
        
        nlohmann::json payload = {
            {"sub", claims.userId},
            {"role", claims.role},
            {"iat", std::chrono::duration_cast<std::chrono::seconds>(claims.issuedAt.time_since_epoch()).count()},
            {"exp", std::chrono::duration_cast<std::chrono::seconds>(claims.expiresAt.time_since_epoch()).count()},
            {"iss", claims.issuer}
        };
        
        std::string encodedHeader = base64UrlEncode(header.dump());
        std::string encodedPayload = base64UrlEncode(payload.dump());
        std::string signature = generateSignature(encodedHeader + "." + encodedPayload);
        
        return encodedHeader + "." + encodedPayload + "." + signature;
    }
    
    std::optional<TokenClaims> validateToken(const std::string& token) {
        auto parts = splitString(token, '.');
        if (parts.size() != 3) {
            return std::nullopt;
        }
        
        // Verify signature
        std::string expectedSignature = generateSignature(parts[0] + "." + parts[1]);
        if (!constantTimeCompare(parts[2], expectedSignature)) {
            LOG_SECURITY_WARNING("Invalid JWT signature detected");
            return std::nullopt;
        }
        
        try {
            // Decode payload
            std::string payloadJson = base64UrlDecode(parts[1]);
            auto payload = nlohmann::json::parse(payloadJson);
            
            // Check expiration
            auto exp = std::chrono::system_clock::from_time_t(payload["exp"]);
            if (std::chrono::system_clock::now() > exp) {
                return std::nullopt;
            }
            
            TokenClaims claims;
            claims.userId = payload["sub"];
            claims.role = payload["role"];
            claims.issuedAt = std::chrono::system_clock::from_time_t(payload["iat"]);
            claims.expiresAt = exp;
            claims.issuer = payload["iss"];
            
            return claims;
            
        } catch (const std::exception& e) {
            LOG_SECURITY_WARNING("JWT parsing error: " + std::string(e.what()));
            return std::nullopt;
        }
    }
    
private:
    std::string generateSignature(const std::string& data) {
        unsigned char hash[32];
        unsigned int hashLen;
        
        HMAC(EVP_sha256(), m_secretKey.c_str(), m_secretKey.length(),
             reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
             hash, &hashLen);
        
        return base64UrlEncode(std::string(reinterpret_cast<char*>(hash), hashLen));
    }
    
    std::string base64UrlEncode(const std::string& input) {
        // Base64 URL encoding implementation
        // ...
        return "";
    }
    
    std::string base64UrlDecode(const std::string& input) {
        // Base64 URL decoding implementation
        // ...
        return "";
    }
    
    std::vector<std::string> splitString(const std::string& str, char delimiter) {
        std::vector<std::string> parts;
        std::stringstream ss(str);
        std::string part;
        
        while (std::getline(ss, part, delimiter)) {
            parts.push_back(part);
        }
        
        return parts;
    }
    
    bool constantTimeCompare(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        volatile int result = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            result |= a[i] ^ b[i];
        }
        
        return result == 0;
    }
    
    std::string m_secretKey;
};
```

## Cryptography

### Encryption and Decryption

```cpp
// AES encryption wrapper
class AESCrypto {
public:
    static constexpr size_t KEY_SIZE = 32; // AES-256
    static constexpr size_t IV_SIZE = 16;
    
    struct EncryptionResult {
        std::vector<unsigned char> ciphertext;
        std::vector<unsigned char> iv;
        std::vector<unsigned char> tag; // For GCM mode
    };
    
    // Generate cryptographically secure key
    static std::vector<unsigned char> generateKey() {
        std::vector<unsigned char> key(KEY_SIZE);
        if (RAND_bytes(key.data(), KEY_SIZE) != 1) {
            throw std::runtime_error("Failed to generate random key");
        }
        return key;
    }
    
    // Generate random IV
    static std::vector<unsigned char> generateIV() {
        std::vector<unsigned char> iv(IV_SIZE);
        if (RAND_bytes(iv.data(), IV_SIZE) != 1) {
            throw std::runtime_error("Failed to generate random IV");
        }
        return iv;
    }
    
    // Encrypt using AES-256-GCM
    static EncryptionResult encrypt(const std::vector<unsigned char>& plaintext,
                                  const std::vector<unsigned char>& key,
                                  const std::vector<unsigned char>& additionalData = {}) {
        if (key.size() != KEY_SIZE) {
            throw std::invalid_argument("Invalid key size");
        }
        
        auto iv = generateIV();
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }
        
        // Initialize encryption
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize encryption");
        }
        
        // Set IV length
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set IV length");
        }
        
        // Initialize key and IV
        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set key and IV");
        }
        
        // Add additional authenticated data
        int len;
        if (!additionalData.empty()) {
            if (EVP_EncryptUpdate(ctx, nullptr, &len, additionalData.data(), additionalData.size()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("Failed to add AAD");
            }
        }
        
        // Encrypt plaintext
        std::vector<unsigned char> ciphertext(plaintext.size());
        if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to encrypt data");
        }
        
        // Finalize encryption
        int finalLen;
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &finalLen) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize encryption");
        }
        
        // Get authentication tag
        std::vector<unsigned char> tag(16);
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to get authentication tag");
        }
        
        EVP_CIPHER_CTX_free(ctx);
        
        ciphertext.resize(len + finalLen);
        return {std::move(ciphertext), std::move(iv), std::move(tag)};
    }
    
    // Decrypt using AES-256-GCM
    static std::vector<unsigned char> decrypt(const EncryptionResult& encrypted,
                                            const std::vector<unsigned char>& key,
                                            const std::vector<unsigned char>& additionalData = {}) {
        if (key.size() != KEY_SIZE) {
            throw std::invalid_argument("Invalid key size");
        }
        
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create cipher context");
        }
        
        // Initialize decryption
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize decryption");
        }
        
        // Set IV length
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, nullptr) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set IV length");
        }
        
        // Initialize key and IV
        if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), encrypted.iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set key and IV");
        }
        
        // Add additional authenticated data
        int len;
        if (!additionalData.empty()) {
            if (EVP_DecryptUpdate(ctx, nullptr, &len, additionalData.data(), additionalData.size()) != 1) {
                EVP_CIPHER_CTX_free(ctx);
                throw std::runtime_error("Failed to add AAD");
            }
        }
        
        // Decrypt ciphertext
        std::vector<unsigned char> plaintext(encrypted.ciphertext.size());
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, encrypted.ciphertext.data(), encrypted.ciphertext.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to decrypt data");
        }
        
        // Set authentication tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, encrypted.tag.size(), 
                               const_cast<unsigned char*>(encrypted.tag.data())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Failed to set authentication tag");
        }
        
        // Finalize decryption and verify tag
        int finalLen;
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &finalLen) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Authentication failed or decryption error");
        }
        
        EVP_CIPHER_CTX_free(ctx);
        
        plaintext.resize(len + finalLen);
        return plaintext;
    }
};
```

### Digital Signatures

```cpp
// Digital signature implementation
class DigitalSignature {
public:
    struct KeyPair {
        EVP_PKEY* publicKey;
        EVP_PKEY* privateKey;
        
        ~KeyPair() {
            if (publicKey) EVP_PKEY_free(publicKey);
            if (privateKey) EVP_PKEY_free(privateKey);
        }
    };
    
    // Generate RSA key pair
    static std::unique_ptr<KeyPair> generateRSAKeyPair(int keySize = 2048) {
        auto keyPair = std::make_unique<KeyPair>();
        
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
        if (!ctx) {
            throw std::runtime_error("Failed to create key generation context");
        }
        
        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize key generation");
        }
        
        if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            throw std::runtime_error("Failed to set key size");
        }
        
        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            throw std::runtime_error("Failed to generate key pair");
        }
        
        keyPair->privateKey = pkey;
        keyPair->publicKey = pkey;
        EVP_PKEY_up_ref(pkey); // Increment reference count
        
        EVP_PKEY_CTX_free(ctx);
        return keyPair;
    }
    
    // Sign data
    static std::vector<unsigned char> sign(const std::vector<unsigned char>& data,
                                         EVP_PKEY* privateKey) {
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create signing context");
        }
        
        if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, privateKey) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize signing");
        }
        
        if (EVP_DigestSignUpdate(ctx, data.data(), data.size()) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to update signing context");
        }
        
        size_t signatureLen;
        if (EVP_DigestSignFinal(ctx, nullptr, &signatureLen) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to determine signature length");
        }
        
        std::vector<unsigned char> signature(signatureLen);
        if (EVP_DigestSignFinal(ctx, signature.data(), &signatureLen) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to create signature");
        }
        
        EVP_MD_CTX_free(ctx);
        signature.resize(signatureLen);
        return signature;
    }
    
    // Verify signature
    static bool verify(const std::vector<unsigned char>& data,
                      const std::vector<unsigned char>& signature,
                      EVP_PKEY* publicKey) {
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            return false;
        }
        
        if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, publicKey) != 1) {
            EVP_MD_CTX_free(ctx);
            return false;
        }
        
        if (EVP_DigestVerifyUpdate(ctx, data.data(), data.size()) != 1) {
            EVP_MD_CTX_free(ctx);
            return false;
        }
        
        int result = EVP_DigestVerifyFinal(ctx, signature.data(), signature.size());
        EVP_MD_CTX_free(ctx);
        
        return result == 1;
    }
};
```

## Secure Communication

### TLS Configuration

```cpp
// Secure TLS client
class SecureTLSClient {
public:
    SecureTLSClient() {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        
        m_ctx = SSL_CTX_new(TLS_client_method());
        if (!m_ctx) {
            throw std::runtime_error("Failed to create SSL context");
        }
        
        // Set minimum TLS version to 1.2
        SSL_CTX_set_min_proto_version(m_ctx, TLS1_2_VERSION);
        
        // Set secure cipher suites
        SSL_CTX_set_cipher_list(m_ctx, "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:DHE+CHACHA20:!aNULL:!MD5:!DSS");
        
        // Enable certificate verification
        SSL_CTX_set_verify(m_ctx, SSL_VERIFY_PEER, nullptr);
        
        // Load CA certificates
        if (SSL_CTX_set_default_verify_paths(m_ctx) != 1) {
            throw std::runtime_error("Failed to load CA certificates");
        }
        
        // Enable hostname verification
        SSL_CTX_set_verify_callback(m_ctx, verifyCallback);
    }
    
    ~SecureTLSClient() {
        if (m_ssl) {
            SSL_shutdown(m_ssl);
            SSL_free(m_ssl);
        }
        if (m_ctx) {
            SSL_CTX_free(m_ctx);
        }
    }
    
    bool connect(const std::string& hostname, int port) {
        // Create socket
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0) {
            return false;
        }
        
        // Connect to server
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, hostname.c_str(), &serverAddr.sin_addr);
        
        if (::connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
            close(m_socket);
            return false;
        }
        
        // Create SSL connection
        m_ssl = SSL_new(m_ctx);
        if (!m_ssl) {
            close(m_socket);
            return false;
        }
        
        // Set hostname for SNI and certificate verification
        SSL_set_tlsext_host_name(m_ssl, hostname.c_str());
        SSL_set1_host(m_ssl, hostname.c_str());
        
        SSL_set_fd(m_ssl, m_socket);
        
        // Perform TLS handshake
        if (SSL_connect(m_ssl) != 1) {
            LOG_SECURITY_ERROR("TLS handshake failed: " + getSSLError());
            return false;
        }
        
        // Verify certificate
        if (SSL_get_verify_result(m_ssl) != X509_V_OK) {
            LOG_SECURITY_ERROR("Certificate verification failed");
            return false;
        }
        
        LOG_SECURITY_INFO("Secure TLS connection established to " + hostname);
        return true;
    }
    
    int send(const std::vector<unsigned char>& data) {
        if (!m_ssl) {
            return -1;
        }
        
        return SSL_write(m_ssl, data.data(), data.size());
    }
    
    std::vector<unsigned char> receive(size_t maxSize = 4096) {
        if (!m_ssl) {
            return {};
        }
        
        std::vector<unsigned char> buffer(maxSize);
        int bytesRead = SSL_read(m_ssl, buffer.data(), maxSize);
        
        if (bytesRead > 0) {
            buffer.resize(bytesRead);
            return buffer;
        }
        
        return {};
    }
    
private:
    static int verifyCallback(int preverifyOk, X509_STORE_CTX* ctx) {
        if (!preverifyOk) {
            char subject[256];
            X509* cert = X509_STORE_CTX_get_current_cert(ctx);
            X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject));
            
            int error = X509_STORE_CTX_get_error(ctx);
            LOG_SECURITY_WARNING("Certificate verification failed for: " + std::string(subject) + 
                               ", error: " + X509_verify_cert_error_string(error));
        }
        
        return preverifyOk;
    }
    
    std::string getSSLError() {
        unsigned long error = ERR_get_error();
        char buffer[256];
        ERR_error_string_n(error, buffer, sizeof(buffer));
        return std::string(buffer);
    }
    
    SSL_CTX* m_ctx = nullptr;
    SSL* m_ssl = nullptr;
    int m_socket = -1;
};
```

## Memory Safety

### Buffer Overflow Prevention

```cpp
// Safe string operations
class SafeString {
public:
    // Safe string copy
    static bool safeCopy(char* dest, size_t destSize, const char* src) {
        if (!dest || !src || destSize == 0) {
            return false;
        }
        
        size_t srcLen = strnlen(src, destSize);
        if (srcLen >= destSize) {
            return false; // Source too long
        }
        
        strncpy(dest, src, destSize - 1);
        dest[destSize - 1] = '\0';
        return true;
    }
    
    // Safe string concatenation
    static bool safeConcat(char* dest, size_t destSize, const char* src) {
        if (!dest || !src || destSize == 0) {
            return false;
        }
        
        size_t destLen = strnlen(dest, destSize);
        if (destLen >= destSize) {
            return false; // Destination already full
        }
        
        size_t remaining = destSize - destLen;
        size_t srcLen = strnlen(src, remaining);
        
        if (srcLen >= remaining) {
            return false; // Not enough space
        }
        
        strncat(dest, src, remaining - 1);
        return true;
    }
    
    // Safe integer to string conversion
    static bool safeIntToString(char* dest, size_t destSize, int value) {
        if (!dest || destSize == 0) {
            return false;
        }
        
        int result = snprintf(dest, destSize, "%d", value);
        return result > 0 && static_cast<size_t>(result) < destSize;
    }
};

// RAII buffer management
template<size_t Size>
class SafeBuffer {
public:
    SafeBuffer() {
        std::fill(m_buffer.begin(), m_buffer.end(), 0);
    }
    
    ~SafeBuffer() {
        // Zero out buffer on destruction
        std::fill(m_buffer.begin(), m_buffer.end(), 0);
    }
    
    bool write(size_t offset, const void* data, size_t dataSize) {
        if (!data || offset >= Size || offset + dataSize > Size) {
            return false;
        }
        
        std::memcpy(m_buffer.data() + offset, data, dataSize);
        return true;
    }
    
    bool read(size_t offset, void* dest, size_t readSize) const {
        if (!dest || offset >= Size || offset + readSize > Size) {
            return false;
        }
        
        std::memcpy(dest, m_buffer.data() + offset, readSize);
        return true;
    }
    
    size_t size() const { return Size; }
    const char* data() const { return m_buffer.data(); }
    
private:
    std::array<char, Size> m_buffer;
};
```

### Integer Overflow Protection

```cpp
// Safe arithmetic operations
class SafeMath {
public:
    // Safe addition
    template<typename T>
    static bool safeAdd(T a, T b, T& result) {
        static_assert(std::is_integral_v<T>, "T must be an integral type");
        
        if constexpr (std::is_signed_v<T>) {
            if (b > 0 && a > std::numeric_limits<T>::max() - b) {
                return false; // Positive overflow
            }
            if (b < 0 && a < std::numeric_limits<T>::min() - b) {
                return false; // Negative overflow
            }
        } else {
            if (a > std::numeric_limits<T>::max() - b) {
                return false; // Unsigned overflow
            }
        }
        
        result = a + b;
        return true;
    }
    
    // Safe multiplication
    template<typename T>
    static bool safeMul(T a, T b, T& result) {
        static_assert(std::is_integral_v<T>, "T must be an integral type");
        
        if (a == 0 || b == 0) {
            result = 0;
            return true;
        }
        
        if constexpr (std::is_signed_v<T>) {
            if (a > 0) {
                if (b > 0 && a > std::numeric_limits<T>::max() / b) {
                    return false;
                }
                if (b < 0 && b < std::numeric_limits<T>::min() / a) {
                    return false;
                }
            } else {
                if (b > 0 && a < std::numeric_limits<T>::min() / b) {
                    return false;
                }
                if (b < 0 && a < std::numeric_limits<T>::max() / b) {
                    return false;
                }
            }
        } else {
            if (a > std::numeric_limits<T>::max() / b) {
                return false;
            }
        }
        
        result = a * b;
        return true;
    }
    
    // Safe array index calculation
    static bool safeArrayAccess(size_t index, size_t arraySize) {
        return index < arraySize;
    }
    
    // Safe size calculation for allocation
    static bool safeSizeForAllocation(size_t elementCount, size_t elementSize, size_t& totalSize) {
        return safeMul(elementCount, elementSize, totalSize) && 
               totalSize <= std::numeric_limits<size_t>::max() / 2; // Leave some headroom
    }
};
```

## Configuration Security

### Secure Configuration Management

```cpp
// Secure configuration with encryption
class SecureConfigManager {
public:
    explicit SecureConfigManager(const std::string& masterKey) {
        // Derive encryption key from master key
        m_encryptionKey = deriveKey(masterKey);
    }
    
    bool setSecureValue(const std::string& key, const std::string& value) {
        try {
            auto encrypted = AESCrypto::encrypt(
                std::vector<unsigned char>(value.begin(), value.end()),
                m_encryptionKey
            );
            
            // Store encrypted value with IV and tag
            SecureValue secureValue;
            secureValue.ciphertext = encrypted.ciphertext;
            secureValue.iv = encrypted.iv;
            secureValue.tag = encrypted.tag;
            
            std::lock_guard<std::mutex> lock(m_mutex);
            m_secureValues[key] = secureValue;
            
            LOG_SECURITY_INFO("Secure value set for key: " + key);
            return true;
            
        } catch (const std::exception& e) {
            LOG_SECURITY_ERROR("Failed to encrypt value for key " + key + ": " + e.what());
            return false;
        }
    }
    
    std::optional<std::string> getSecureValue(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_secureValues.find(key);
        if (it == m_secureValues.end()) {
            return std::nullopt;
        }
        
        try {
            AESCrypto::EncryptionResult encrypted;
            encrypted.ciphertext = it->second.ciphertext;
            encrypted.iv = it->second.iv;
            encrypted.tag = it->second.tag;
            
            auto decrypted = AESCrypto::decrypt(encrypted, m_encryptionKey);
            return std::string(decrypted.begin(), decrypted.end());
            
        } catch (const std::exception& e) {
            LOG_SECURITY_ERROR("Failed to decrypt value for key " + key + ": " + e.what());
            return std::nullopt;
        }
    }
    
    // Validate configuration against schema
    bool validateConfiguration(const nlohmann::json& config, const nlohmann::json& schema) {
        // Basic schema validation
        for (const auto& [key, schemaValue] : schema.items()) {
            if (!config.contains(key)) {
                if (schemaValue.contains("required") && schemaValue["required"].get<bool>()) {
                    LOG_SECURITY_ERROR("Required configuration key missing: " + key);
                    return false;
                }
                continue;
            }
            
            const auto& configValue = config[key];
            
            // Type validation
            if (schemaValue.contains("type")) {
                std::string expectedType = schemaValue["type"];
                if (!validateType(configValue, expectedType)) {
                    LOG_SECURITY_ERROR("Invalid type for configuration key: " + key);
                    return false;
                }
            }
            
            // Range validation
            if (schemaValue.contains("min") && configValue.is_number()) {
                if (configValue.get<double>() < schemaValue["min"].get<double>()) {
                    LOG_SECURITY_ERROR("Value below minimum for key: " + key);
                    return false;
                }
            }
            
            if (schemaValue.contains("max") && configValue.is_number()) {
                if (configValue.get<double>() > schemaValue["max"].get<double>()) {
                    LOG_SECURITY_ERROR("Value above maximum for key: " + key);
                    return false;
                }
            }
        }
        
        return true;
    }
    
private:
    struct SecureValue {
        std::vector<unsigned char> ciphertext;
        std::vector<unsigned char> iv;
        std::vector<unsigned char> tag;
    };
    
    std::vector<unsigned char> deriveKey(const std::string& masterKey) {
        // Use PBKDF2 to derive encryption key
        std::string salt = "CoreBaseApplication_Salt_2024"; // Use a proper random salt in production
        
        std::vector<unsigned char> derivedKey(32);
        PKCS5_PBKDF2_HMAC(masterKey.c_str(), masterKey.length(),
                         reinterpret_cast<const unsigned char*>(salt.c_str()), salt.length(),
                         100000, EVP_sha256(), 32, derivedKey.data());
        
        return derivedKey;
    }
    
    bool validateType(const nlohmann::json& value, const std::string& expectedType) {
        if (expectedType == "string") return value.is_string();
        if (expectedType == "number") return value.is_number();
        if (expectedType == "boolean") return value.is_boolean();
        if (expectedType == "array") return value.is_array();
        if (expectedType == "object") return value.is_object();
        return false;
    }
    
    std::mutex m_mutex;
    std::vector<unsigned char> m_encryptionKey;
    std::unordered_map<std::string, SecureValue> m_secureValues;
};
```

## Logging and Monitoring

### Security Event Logging

```cpp
// Security-focused logging
class SecurityLogger {
public:
    enum class SecurityEventType {
        AUTHENTICATION_SUCCESS,
        AUTHENTICATION_FAILURE,
        AUTHORIZATION_FAILURE,
        SUSPICIOUS_ACTIVITY,
        SECURITY_VIOLATION,
        PRIVILEGE_ESCALATION,
        DATA_ACCESS,
        CONFIGURATION_CHANGE
    };
    
    struct SecurityEvent {
        SecurityEventType type;
        std::string userId;
        std::string clientIP;
        std::string resource;
        std::string action;
        std::string details;
        std::chrono::system_clock::time_point timestamp;
    };
    
    static SecurityLogger& getInstance() {
        static SecurityLogger instance;
        return instance;
    }
    
    void logSecurityEvent(const SecurityEvent& event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Create structured log entry
        nlohmann::json logEntry = {
            {"timestamp", std::chrono::duration_cast<std::chrono::seconds>(event.timestamp.time_since_epoch()).count()},
            {"type", securityEventTypeToString(event.type)},
            {"user_id", sanitizeForLogging(event.userId)},
            {"client_ip", sanitizeForLogging(event.clientIP)},
            {"resource", sanitizeForLogging(event.resource)},
            {"action", sanitizeForLogging(event.action)},
            {"details", sanitizeForLogging(event.details)}
        };
        
        // Write to security log file
        m_securityLogFile << logEntry.dump() << std::endl;
        m_securityLogFile.flush();
        
        // Check for security alerts
        checkForSecurityAlerts(event);
        
        // Store in memory for analysis
        m_recentEvents.push_back(event);
        if (m_recentEvents.size() > MAX_RECENT_EVENTS) {
            m_recentEvents.pop_front();
        }
    }
    
    void logAuthenticationFailure(const std::string& userId, const std::string& clientIP, const std::string& reason) {
        SecurityEvent event;
        event.type = SecurityEventType::AUTHENTICATION_FAILURE;
        event.userId = userId;
        event.clientIP = clientIP;
        event.action = "login_attempt";
        event.details = reason;
        event.timestamp = std::chrono::system_clock::now();
        
        logSecurityEvent(event);
    }
    
    void logSuspiciousActivity(const std::string& userId, const std::string& clientIP, const std::string& activity) {
        SecurityEvent event;
        event.type = SecurityEventType::SUSPICIOUS_ACTIVITY;
        event.userId = userId;
        event.clientIP = clientIP;
        event.action = "suspicious_activity";
        event.details = activity;
        event.timestamp = std::chrono::system_clock::now();
        
        logSecurityEvent(event);
    }
    
    // Analyze recent events for patterns
    std::vector<std::string> analyzeSecurityPatterns() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> alerts;
        
        // Check for brute force attacks
        std::unordered_map<std::string, int> failuresByIP;
        auto now = std::chrono::system_clock::now();
        auto oneHourAgo = now - std::chrono::hours(1);
        
        for (const auto& event : m_recentEvents) {
            if (event.type == SecurityEventType::AUTHENTICATION_FAILURE && event.timestamp > oneHourAgo) {
                failuresByIP[event.clientIP]++;
            }
        }
        
        for (const auto& [ip, failures] : failuresByIP) {
            if (failures >= 10) {
                alerts.push_back("Potential brute force attack from IP: " + ip + " (" + std::to_string(failures) + " failures)");
            }
        }
        
        return alerts;
    }
    
private:
    SecurityLogger() {
        m_securityLogFile.open("security.log", std::ios::app);
    }
    
    std::string securityEventTypeToString(SecurityEventType type) {
        switch (type) {
            case SecurityEventType::AUTHENTICATION_SUCCESS: return "AUTH_SUCCESS";
            case SecurityEventType::AUTHENTICATION_FAILURE: return "AUTH_FAILURE";
            case SecurityEventType::AUTHORIZATION_FAILURE: return "AUTHZ_FAILURE";
            case SecurityEventType::SUSPICIOUS_ACTIVITY: return "SUSPICIOUS";
            case SecurityEventType::SECURITY_VIOLATION: return "VIOLATION";
            case SecurityEventType::PRIVILEGE_ESCALATION: return "PRIVILEGE_ESC";
            case SecurityEventType::DATA_ACCESS: return "DATA_ACCESS";
            case SecurityEventType::CONFIGURATION_CHANGE: return "CONFIG_CHANGE";
            default: return "UNKNOWN";
        }
    }
    
    std::string sanitizeForLogging(const std::string& input) {
        std::string result = input;
        
        // Remove or escape dangerous characters
        std::replace(result.begin(), result.end(), '\n', ' ');
        std::replace(result.begin(), result.end(), '\r', ' ');
        std::replace(result.begin(), result.end(), '\t', ' ');
        
        // Limit length
        if (result.length() > 1000) {
            result = result.substr(0, 1000) + "...";
        }
        
        return result;
    }
    
    void checkForSecurityAlerts(const SecurityEvent& event) {
        // Implement real-time alerting logic
        if (event.type == SecurityEventType::SECURITY_VIOLATION ||
            event.type == SecurityEventType::PRIVILEGE_ESCALATION) {
            // Send immediate alert
            sendSecurityAlert(event);
        }
    }
    
    void sendSecurityAlert(const SecurityEvent& event) {
        // Implementation for sending alerts (email, SMS, etc.)
        LOG_CRITICAL("SECURITY ALERT: " + securityEventTypeToString(event.type) + 
                    " - User: " + event.userId + ", IP: " + event.clientIP);
    }
    
    static constexpr size_t MAX_RECENT_EVENTS = 10000;
    std::mutex m_mutex;
    std::ofstream m_securityLogFile;
    std::deque<SecurityEvent> m_recentEvents;
};
```

## Dependency Management

### Secure Dependency Handling

```cpp
// Dependency verification
class DependencyManager {
public:
    struct DependencyInfo {
        std::string name;
        std::string version;
        std::string checksum;
        std::string source;
        bool verified = false;
    };
    
    // Verify dependency integrity
    static bool verifyDependency(const DependencyInfo& dependency, const std::string& filePath) {
        try {
            // Calculate file checksum
            std::string calculatedChecksum = calculateSHA256(filePath);
            
            // Compare with expected checksum
            if (calculatedChecksum != dependency.checksum) {
                LOG_SECURITY_ERROR("Checksum mismatch for dependency: " + dependency.name);
                return false;
            }
            
            // Verify digital signature if available
            if (!verifySignature(filePath, dependency)) {
                LOG_SECURITY_WARNING("Signature verification failed for: " + dependency.name);
                return false;
            }
            
            LOG_SECURITY_INFO("Dependency verified: " + dependency.name + " v" + dependency.version);
            return true;
            
        } catch (const std::exception& e) {
            LOG_SECURITY_ERROR("Error verifying dependency " + dependency.name + ": " + e.what());
            return false;
        }
    }
    
    // Check for known vulnerabilities
    static std::vector<std::string> checkVulnerabilities(const std::vector<DependencyInfo>& dependencies) {
        std::vector<std::string> vulnerabilities;
        
        // Load vulnerability database
        auto vulnDB = loadVulnerabilityDatabase();
        
        for (const auto& dep : dependencies) {
            auto vulns = vulnDB.getVulnerabilities(dep.name, dep.version);
            for (const auto& vuln : vulns) {
                vulnerabilities.push_back(dep.name + " v" + dep.version + ": " + vuln);
            }
        }
        
        return vulnerabilities;
    }
    
private:
    static std::string calculateSHA256(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file: " + filePath);
        }
        
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            throw std::runtime_error("Failed to create hash context");
        }
        
        if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to initialize hash");
        }
        
        char buffer[8192];
        while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
            if (EVP_DigestUpdate(ctx, buffer, file.gcount()) != 1) {
                EVP_MD_CTX_free(ctx);
                throw std::runtime_error("Failed to update hash");
            }
        }
        
        unsigned char hash[32];
        unsigned int hashLen;
        if (EVP_DigestFinal_ex(ctx, hash, &hashLen) != 1) {
            EVP_MD_CTX_free(ctx);
            throw std::runtime_error("Failed to finalize hash");
        }
        
        EVP_MD_CTX_free(ctx);
        
        // Convert to hex string
        std::stringstream ss;
        for (unsigned int i = 0; i < hashLen; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        
        return ss.str();
    }
    
    static bool verifySignature(const std::string& filePath, const DependencyInfo& dependency) {
        // Implementation depends on signature format and public key management
        // This is a simplified example
        return true;
    }
    
    static VulnerabilityDatabase loadVulnerabilityDatabase() {
        // Load from CVE database or security advisory feeds
        return VulnerabilityDatabase{};
    }
};
```

## Security Testing

### Automated Security Tests

```cpp
// Security test framework
class SecurityTests {
public:
    static void runAllSecurityTests() {
        LOG_INFO("Starting security test suite...");
        
        testInputValidation();
        testAuthenticationSecurity();
        testAuthorizationControls();
        testCryptographicFunctions();
        testMemorySafety();
        testConfigurationSecurity();
        
        LOG_INFO("Security test suite completed.");
    }
    
private:
    static void testInputValidation() {
        LOG_INFO("Testing input validation...");
        
        // Test SQL injection prevention
        assert(InputValidator::validateSqlInput("'; DROP TABLE users; --").error == 
               InputValidator::ValidationError::CONTAINS_MALICIOUS_CONTENT);
        
        // Test XSS prevention
        std::string xssInput = "<script>alert('xss')</script>";
        std::string sanitized = InputValidator::sanitizeHtml(xssInput);
        assert(sanitized.find("<script>") == std::string::npos);
        
        // Test buffer overflow prevention
        std::string longInput(10000, 'A');
        assert(!InputValidator::validateEmail(longInput).isValid());
        
        LOG_INFO("Input validation tests passed.");
    }
    
    static void testAuthenticationSecurity() {
        LOG_INFO("Testing authentication security...");
        
        // Test password hashing
        std::string password = "TestPassword123!";
        auto hashed = PasswordManager::hashPassword(password);
        assert(PasswordManager::verifyPassword(password, hashed));
        assert(!PasswordManager::verifyPassword("WrongPassword", hashed));
        
        // Test password strength
        assert(PasswordManager::calculatePasswordStrength("weak") < 5);
        assert(PasswordManager::calculatePasswordStrength("StrongP@ssw0rd123") >= 7);
        
        LOG_INFO("Authentication security tests passed.");
    }
    
    static void testAuthorizationControls() {
        LOG_INFO("Testing authorization controls...");
        
        AccessControl::initializeRolePermissions();
        
        AccessControl::User guestUser("guest", AccessControl::Role::GUEST);
        AccessControl::User adminUser("admin", AccessControl::Role::ADMIN);
        
        assert(!AccessControl::checkPermission(guestUser, AccessControl::Permission::WRITE));
        assert(AccessControl::checkPermission(adminUser, AccessControl::Permission::WRITE));
        
        LOG_INFO("Authorization control tests passed.");
    }
    
    static void testCryptographicFunctions() {
        LOG_INFO("Testing cryptographic functions...");
        
        // Test AES encryption/decryption
        std::string plaintext = "This is a test message for encryption.";
        auto key = AESCrypto::generateKey();
        
        auto encrypted = AESCrypto::encrypt(
            std::vector<unsigned char>(plaintext.begin(), plaintext.end()), key);
        
        auto decrypted = AESCrypto::decrypt(encrypted, key);
        std::string decryptedText(decrypted.begin(), decrypted.end());
        
        assert(plaintext == decryptedText);
        
        LOG_INFO("Cryptographic function tests passed.");
    }
    
    static void testMemorySafety() {
        LOG_INFO("Testing memory safety...");
        
        // Test safe buffer operations
        SafeBuffer<100> buffer;
        std::string testData = "Test data";
        
        assert(buffer.write(0, testData.c_str(), testData.length()));
        assert(!buffer.write(95, testData.c_str(), testData.length())); // Should fail
        
        // Test safe arithmetic
        int result;
        assert(SafeMath::safeAdd(100, 200, result) && result == 300);
        assert(!SafeMath::safeAdd(std::numeric_limits<int>::max(), 1, result));
        
        LOG_INFO("Memory safety tests passed.");
    }
    
    static void testConfigurationSecurity() {
        LOG_INFO("Testing configuration security...");
        
        SecureConfigManager configManager("test_master_key_12345678901234567890");
        
        std::string secretValue = "sensitive_data_123";
        assert(configManager.setSecureValue("test_key", secretValue));
        
        auto retrieved = configManager.getSecureValue("test_key");
        assert(retrieved.has_value() && retrieved.value() == secretValue);
        
        LOG_INFO("Configuration security tests passed.");
    }
};
```

## Best Practices Summary

### Security Checklist

- [ ] **Input Validation**: All user inputs are validated and sanitized
- [ ] **Authentication**: Strong password policies and secure authentication mechanisms
- [ ] **Authorization**: Proper access controls and principle of least privilege
- [ ] **Encryption**: Sensitive data encrypted at rest and in transit
- [ ] **Secure Communication**: TLS 1.2+ for all network communications
- [ ] **Memory Safety**: Buffer overflow protection and safe memory management
- [ ] **Error Handling**: Secure failure modes that don't leak information
- [ ] **Logging**: Comprehensive security event logging and monitoring
- [ ] **Dependencies**: Regular security updates and vulnerability scanning
- [ ] **Testing**: Automated security tests in CI/CD pipeline

### Security Resources

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [CWE/SANS Top 25](https://cwe.mitre.org/top25/)
- [NIST Cybersecurity Framework](https://www.nist.gov/cyberframework)
- [ISO 27001](https://www.iso.org/isoiec-27001-information-security.html)

### Regular Security Tasks

1. **Weekly**: Review security logs and alerts
2. **Monthly**: Update dependencies and scan for vulnerabilities
3. **Quarterly**: Conduct security assessments and penetration testing
4. **Annually**: Review and update security policies and procedures

---

*This security guide should be regularly updated to reflect new threats and best practices. Always stay informed about the latest security vulnerabilities and mitigation strategies.*