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
  - **Use range-based for `for (auto& e : v)` instead of `for (int i = 0; i < n; i++) `, whenever it is possible**.
- **Declaration of namespace**:
  - Should be minimum to prevent conflicts.
  - Defined in each `cpp` files. Do **not** write it in `hpp` files to prevent spreading of the declaration.
  - Do **not** declare entire name space. `using namespace std; // not good`
  - You can declare a function if that's commonly used. `using std::cout; // not bad`
  - You are **recommended** to rename long namespace, instead of declaring it. `namespace fs = std::filesystem; // very good`
  - Currently approved declarations:
    - `using std::cout;`
    - `using std::endl;`
    - `namespace fs = std::filesystem;`
    - `namespace chrono = std::chrono;`
  - Curretnly **prohibited** declarations:
    - `using namespace std;`
    - `using namespace Eigen;`: to let beginers know Eigen is used
    - `using std::vector`: not to be confused with `Vector` in Eigen

## Issue Creation

Please create an issue before starting to work on any feature or bug fix.

## Commit Messages and Pull Request Titles

There are no strict requirements for commit messages or pull request titles. Pull requests will be squashed and merged.

## Licensing

This project is licensed under the MIT License.

## Communication

Communication for this project primarily takes place on GitHub.
