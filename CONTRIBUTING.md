# Contributing to MPS-Basic

Thank you for considering contributing to MPS-Basic! We appreciate your interest and support in making this project better.

## Contribution Workflow

1. Create an issue and assign yourself.
2. Clone the repository and make your changes.
3. Create a new branch with the issue number included in the branch name in the format `1-fix-description`.
4. Submit a pull request. Include references to the issue in the pull request description. Assign yourself as the assignee.
5. Your pull request will be reviewed and merged if at least one maintainer approves it.

## Guidelines

- **C++ Standard**: We adhere to C++17.
- **Exceptions**: Do **NOT** use exceptions.
- **Macros**:
  - Defined in `common.hpp`.
  - Excessive use should be avoided.
  - You can use `rep(i, a, b)` if you need to write `for (int i = a; i < b; i++)`.
    - However, **use range-based for `for (auto& e : v)` instead whenever it is possible**.

## Issue Creation

Please create an issue before starting to work on any feature or bug fix.

## Commit Messages and Pull Request Titles

There are no strict requirements for commit messages or pull request titles. Pull requests will be squashed and merged.

## Licensing

This project is licensed under the MIT License.

## Communication

Communication for this project primarily takes place on GitHub.
