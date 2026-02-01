# CLAUDE.md (C++)

## Primary Rule
During refactoring, behavior MUST remain identical. No logic removal, no weakened edge cases, no silent semantic changes. Improvements must not introduce avoidable inefficiency.

## Definitions
- A component is a cohesive set of .h/.cpp files.
- A package is a cohesive set of components.


## Design
- Layer libraries by dependency weight (pure language → minimal framework → full GUI). Lower layers cannot depend on heavier externals. Reusable libraries have zero application knowledge.
- Code must be modular and distributed in adeguate libs (base core gui etc with meaningful short names)
- all main files must be short and call library code. Basically almost no code there. 
- All code must be written to be easily testable
- GUI libs and code must be not much, as little as possible, most of code should go in non-gui libs
- Use as few external dependencies as possible
- Prefer clear architecture: layers (API, domain, infra), explicit boundaries, minimal coupling.
- Define stable interfaces
- Use dependency inversion: depend on abstractions (ok to use also concepts or virtual in non performant paths, even if better to make all performant)
- Do minimal includes; use forward declarations;
- ENFORCE an acyclic dependency graph between components and packages
- Design for change isolation.
- BE CONSISTENT 
- Structure libraries so that they depend on as few external dependencies as possible.


## Dependencies
- Before adding a third party library dependency ask me and before that tell me which license the third party is
- Prefer dynamic linking, especially for GPL libraries
- Do not modify GPL libraries without asking me

## Refactoring without behavior change
- Preserve outputs, side effects, error handling, and relevant ordering.
- If requirements are unclear, do not guess. Isolate uncertainty and make assumptions explicit.
- Do not delete existing comments. Fix them if incorrect.

## Small functions and small classes
- Keep functions short and single-purpose.
- Extract helpers and introduce small domain types with meaningful names.
- If a class grows, separate concerns: parsing/IO, policy, data, orchestration.
- be concise but write code that can be understood by me and efficient

## Naming
- Use names that carry intent and units: `bytesRead`, `retryBudget`, `headerLength`.
- Avoid unexplained abbreviations. If domain acronyms exist, apply consistently.
- Comments: explain rationale and invariants;

## Efficiency is always considered
- Avoid copies/allocations: refs, moves, `string_view`/`span` where ownership is not needed.
- Avoid expensive work in loops and repeated conversions/parsing.
- Keep lock scope minimal; avoid work while holding locks.
- If a change may impact performance, state the tradeoff and how to measure it.

## Minimal C++ rules
- RAII and smart pointers. No raw ownership.
- Const-correctness and clear lifetimes.
- Prefer standard types and `enum class` / `nullptr`.
- One error-handling strategy per module unless explicitly justified.

## Delivery standard
- Provide a change list with rationale, impact, and risks.
- Specify tests to add/update.
- Preserve existing style. Use Whitesmith indenting for new/edited C++ blocks.
- Touch only what is needed; avoid mass reformatting.

## Code structure
- 

## Workflow
- After completing a feature: compile, run existing tests (if any), then commit and push.
- Format: one entry per code area, listing related files/classes and the tests to run (test file path + command).
- `TEST_INFO.txt` is the source of truth mapping code areas to tests.
- When adding or modifying tests, update `TEST_INFO.txt` in the same change.
- When modifying code, consult `TEST_INFO.txt` to determine which tests must be updated and executed.

## Compilation
- For Qt I usually use: /home/daniele/installers/qt/Tools/CMake/bin/cmake with build: /home/daniele/installers/qt/6.10.1/gcc_64

## Example code for new code or for refactoring code well:

### .h
#ifndef MY_CLASS_H
#define MY_CLASS_H

namespace ds:: inline library
    {
    enum class MyEnumClass
        {
        invalid=0,
        typeOne,
        typeTwo
        };

    const int globalVariableK_g;

    struct MyClassInput
      {
      std::string path="someDefaultPath";
      int myOption=1;
      };
      
    class MyClass
      {
      public:
          MyClass() = default;
          MyClass(const MyClassInput& input);
          void setValue(bool value);
          void initialize(const MyClassInput& input);
      private:
          // methods first, members after
          void doInitialize(const MyClassInput& input);
          void privateMethod();
          const int myInstanceConstantVariableK_ = 0;
          int myInstanceVariable_ = 0;
          bool value_ = false;
          MyClassInput input_;
      };
    }

#endif // MY_CLASS_H

### .cpp

#include "test_to_move.h"

using namespace ds::library;

MyClass::MyClass(const MyClassInput& input),
  input_(input)
  {
  doInitialize(input);
  }
  
void MyClass::initialize(const MyClassInput& input)
  {
  input_ = input;
  doInitialize(input);
  }

void MyClass::doInitialize(const MyClassInput& input)
  {
  // do Something for initialization
  }
  
// @note use these kind of comments
void MyClass::setValue(bool value)
  {
  value_ = value;
  }

void MyClass::privateMethod()
  {
  // do something
  }

### filenames
my_file.h my_file.cpp
