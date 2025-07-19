# Contributing to CoreBaseApplication (CBA)

We love your input! We want to make contributing to CoreBaseApplication as easy and transparent as possible, whether it's:

- Reporting a bug
- Discussing the current state of the code
- Submitting a fix
- Proposing new features
- Becoming a maintainer

## Development Process

We use GitHub to host code, to track issues and feature requests, as well as accept pull requests.

## Pull Requests

Pull requests are the best way to propose changes to the codebase. We actively welcome your pull requests:

1. Fork the repo and create your branch from `main`.
2. If you've added code that should be tested, add tests.
3. If you've changed APIs, update the documentation.
4. Ensure the test suite passes.
5. Make sure your code lints.
6. Issue that pull request!

## Code of Conduct

By participating in this project, you agree to abide by our [Code of Conduct](CODE_OF_CONDUCT.md).

## Development Setup

See our [Development Setup Guide](docs/development/setup.md) for detailed instructions on setting up your development environment.

### Quick Setup

```bash
# Clone your fork
git clone https://github.com/seregonwar/CoreBaseApplication-CBA.git
cd CoreBaseApplication-CBA

# Create a new branch
git checkout -b feature/seregonwar

# Build the project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCBA_ENABLE_TESTS=ON
make -j$(nproc)

# Run tests
ctest --output-on-failure
```

## Coding Standards

We follow strict coding standards to maintain code quality and consistency. Please review our [Best Practices Guide](docs/development/best-practices.md) before contributing.

### Key Guidelines

- **C++ Standard**: Use C++17 features appropriately
- **Naming Conventions**: 
  - Classes: `PascalCase`
  - Functions/Methods: `camelCase`
  - Variables: `camelCase`
  - Constants: `UPPER_SNAKE_CASE`
  - Private members: trailing underscore `member_`
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: Maximum 100 characters
- **Documentation**: All public APIs must have Doxygen comments

### Code Style Example

```cpp
/**
 * @brief Manages HTTP client connections and requests
 * 
 * The HttpClient class provides a high-level interface for making
 * HTTP requests with support for connection pooling, retries, and
 * various authentication methods.
 */
class HttpClient {
public:
    /**
     * @brief Constructs an HttpClient with default configuration
     */
    HttpClient();
    
    /**
     * @brief Performs a GET request to the specified URL
     * @param url The target URL
     * @param headers Optional HTTP headers
     * @return HttpResponse containing the server response
     * @throws NetworkException if the request fails
     */
    HttpResponse get(const std::string& url, 
                    const HeaderMap& headers = {}) const;

private:
    std::unique_ptr<HttpClientImpl> impl_;
    mutable std::mutex mutex_;
};
```

## Testing

We use Google Test for unit testing and integration testing. All new features must include comprehensive tests.

### Writing Tests

```cpp
#include <gtest/gtest.h>
#include "HttpClient.h"

class HttpClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        client_ = std::make_unique<HttpClient>();
    }
    
    std::unique_ptr<HttpClient> client_;
};

TEST_F(HttpClientTest, GetRequestReturnsValidResponse) {
    // Arrange
    const std::string url = "https://httpbin.org/get";
    
    // Act
    auto response = client_->get(url);
    
    // Assert
    EXPECT_TRUE(response.isSuccess());
    EXPECT_EQ(response.statusCode(), 200);
    EXPECT_FALSE(response.body().empty());
}

TEST_F(HttpClientTest, InvalidUrlThrowsException) {
    // Arrange
    const std::string invalidUrl = "not-a-valid-url";
    
    // Act & Assert
    EXPECT_THROW(client_->get(invalidUrl), NetworkException);
}
```

### Running Tests

```bash
# Run all tests
ctest --output-on-failure

# Run specific test suite
ctest -R HttpClientTest

# Run tests with verbose output
ctest --verbose

# Generate coverage report
make coverage
```

## Documentation

Documentation is crucial for the project's success. We use:

- **Doxygen** for API documentation
- **Markdown** for guides and tutorials
- **Mermaid** for diagrams

### Documentation Guidelines

1. **API Documentation**: All public classes, methods, and functions must have Doxygen comments
2. **User Guides**: Update relevant guides when adding new features
3. **Examples**: Provide practical examples for new functionality
4. **Architecture**: Update architecture documentation for significant changes

### Building Documentation

```bash
# Generate API documentation
make docs

# Serve documentation locally
cd docs
bundle exec jekyll serve
```

## Submitting Issues

### Bug Reports

Great bug reports tend to have:

- A quick summary and/or background
- Steps to reproduce
  - Be specific!
  - Give sample code if you can
- What you expected would happen
- What actually happens
- Notes (possibly including why you think this might be happening, or stuff you tried that didn't work)

### Feature Requests

We welcome feature requests! Please provide:

- **Use Case**: Describe the problem you're trying to solve
- **Proposed Solution**: How you envision the feature working
- **Alternatives**: Other solutions you've considered
- **Additional Context**: Any other relevant information

## Commit Message Guidelines

We follow the [Conventional Commits](https://www.conventionalcommits.org/) specification:

```
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

### Types

- `feat`: A new feature
- `fix`: A bug fix
- `docs`: Documentation only changes
- `style`: Changes that do not affect the meaning of the code
- `refactor`: A code change that neither fixes a bug nor adds a feature
- `perf`: A code change that improves performance
- `test`: Adding missing tests or correcting existing tests
- `chore`: Changes to the build process or auxiliary tools

### Examples

```
feat(http): add support for custom headers in HttpClient

fix(logging): resolve memory leak in RotatingFileAppender

docs(api): update HttpClient documentation with examples

test(config): add unit tests for ConfigManager validation
```

## Release Process

1. **Version Bump**: Update version numbers in relevant files
2. **Changelog**: Update CHANGELOG.md with new features and fixes
3. **Documentation**: Ensure all documentation is up to date
4. **Testing**: Run full test suite including integration tests
5. **Tag**: Create a git tag with the version number
6. **Release**: Create a GitHub release with release notes

## Getting Help

If you need help with contributing:

- 💬 **Discord**: [Join our community](https://discord.gg/cba-community)
- 📧 **Email**: development@corebaseapp.com
- 📖 **Documentation**: [Development Guide](docs/development/setup.md)
- 🐛 **Issues**: [GitHub Issues](https://github.com/your-org/CoreBaseApplication-CBA/issues)

## Recognition

Contributors are recognized in:

- **CONTRIBUTORS.md**: List of all contributors
- **Release Notes**: Major contributions are highlighted
- **Documentation**: Author credits where appropriate

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to CoreBaseApplication! 🎉