# Coding Techniques

[TOC]

## Interface Class
### What it is
We want to prepare several pressure calculation methods for the user to choose from.
But so as not to interfere with other parts of the code,
they should all have a common input and output,
such as taking particles as input and returning pressure as output.
Otherwise, switching between classes requires changing their inputs
and handling their outputs, which is troublesome.
This problem is not limited to pressure calculation methods,
but is always a problem when trying to provide alternative methods.
How can we define multiple classes,
but guarantee that the inputs and outputs of each class are identical
to the other classes?
The interface class is useful in such situations.

Let's take a look at the example of pressure calculation.
PressureCalculator::Interface class defines
how all the pressure calculator class should act.
```cpp
namespace PressureCalculator {
class Interface {
public:
    virtual std::vector<double> calc(const std::vector<Particle>& particles) = 0;

    virtual ~Interface(){};
};
}
```
A virtual function is a member function that is declared in a base class
and is potentially overridden (redefined) in derived classes.
Declaring a virtual function as `= 0` makes it a pure virtual function,
meaning derived classes must override this function.
In the above example, all the classes that's derived from `Interface` class
***must*** override `calc` function, because it's a pure virtual function.
At the same time,
those new `calc` functions must receive `vector<Particles>` as an input,
and return `vector<double>` as an output.
If you take a look at PressureCalculator::Implicit::calc(), for example,
you can see it follows the rule defined in this `Interface` class.
Therefore, there is no need for us to modify any other parts of the code
even when we change pressure calculation class from
`PressureCalculator::Implicit` to `PressureCalculator::Explicit`, for example.

`virtual ~Interface(){};` is a destructor.
It doesn't have any process in this code, but it's recommended to declare it here
by c++ rule.

So in short,
- Interface class defines the input and output of functions
  that its derived class must have.
- By deriving new class from the interface class, we can guarantee
  that all the derived classes have same input and output.
  Therefore there is no need for us to modify other parts of the code
  even when we switch which class to use.

### How to Use
As an example, we define `Animal` class that has `speak()` function
as a pure virtual function.
Two classes, `Dog` and `Cat`, are derived from `Animal` class,
and override `speak()` function.
```cpp
class Animal {
public:
    virtual void speak() = 0; // pure virtual fucntion
    virtual ~Animal() {}      // virtual destructor
};

class Dog : public Animal {
public:
    void speak() override {
        std::cout << "Bark!" << std::endl;
    }
};

class Cat : public Animal {
public:
    void speak() override {
        std::cout << "Meow!" << std::endl;
    }
};
```

In the main program, first we define `Animal` pointer.
Then it's assigned to new instance of `Dog` or `Cat`.
```cpp
int main() {
    // input data
    string animalType = "Dog";

    // create an Animal pointer
    Animal* myAnimal;

    // Create an instance of Dog or Cat and make myAnimal refer to it
    if (animalType == "Dog"){
      myAnimal = new Dog();

    } else if (animalType == "Cat"){
      myAnimal = new Cat();
    }

    myAnimal->speak();  // output: Bark! or Meow!
    delete myAnimal;

    return 0;
}
```

The reason why we first defined a pointer is because in c++ it is the only way
to dynamically determine the entity of a variable.
Since the `Animal` class works as a class just like the `Dog` and `Cat` classes,
if you define `myAnimal` as `Animal` class,
then `myAnimal` will only work as an `Animal` class, not `Dog` or `Cat`.
If we use a pointer,
- We can change where the pointer refers,
  meaning we can determine the entity of the variable dynamically
- The compiler knows `myAnimal` is `Animal` class, allowing us to call functions
  defined in `Animal` class
  (which is also defined in either `Dog` or `Cat` class since we use pure virtual function).

@attention
This way of pointer usage is dangerous and not recommended.
See [Smart Pointer](#smart-pointer) for more information.

## Smart Pointer {#smart-pointer}
### Using Raw Pointers
Using raw pointers is ***dangerous*** because:
- We must call `delete` to deallocate memory ***manually*** to prevent memory leaks.
- Even if we call `delete`, throwing exceptions or returning early from functions
  can prevent the code from reaching the `delete`, leading to memory leaks.

```cpp
int main(){
  MyClass* myObject = new MyClass(); // Dynamic allocation of object

  // Some operations

  delete myObject; // Manual memory deallocation

  return 0;
}
```

### Using Smart Pointers
From c++11, smart pointers have been introduced into standard libraries.
Smart pointers allow us to use pointers safely because:
- We don't have to call `delete`, but the smart pointer does it automatically.
- The pointer is still automatically deallocated the memory in the event of an exception.
```cpp
#include <memory>

int main(){
  std::unique_ptr<MyClass> myObject(new MyClass()); // Dynamic allocation of object using unique pointer

  // Some operations

  return 0; // Memory is automatically deallocated
}
```

@note
There are other smart pointers such as `shared_ptr` and `weak_ptr`,
but we don't cover them here.

### Ownership Transfer of Unique Pointer
Unique pointer is the sole owner of the object,
meaning other pointers cannot point the same object as the unique pointer.
Therefore, the code bellow causes a compile error.
```cpp
std::unique_ptr<MyClass> ptr1(new MyClass());
std::unique_ptr<MyClass> ptr2 = ptr1; // Compile error
```

To transfer the ownership of the unique pointer,
meaning to change the owner of the object from a unique pointer to a different pointer,
we can use `std::move`.
```cpp
std::unique_ptr<MyClass> ptr1(new MyClass());
std::unique_ptr<MyClass> ptr2 = std::move(ptr1);
// ptr1 now points to nothing.
// Ownership has been transferred to ptr2
```

This way of transferring ownership is especially useful
when we want to use a smart pointer for a function argument.
```cpp
void myFunc(std::unique_ptr<MyClass> ptr){
  // Some operations
}

std::unique_ptr<MyClass> myPtr(new MyClass());
myFunc(myPtr);            // Compile error: conflict of ownership
myFunc(std::move(myPtr)); // GOOD
```
In the above example,
`myFunc(myPtr)` doesn't work because `ptr` in `myFunc` requires an ownership
that `myPtr` has.
`myFunc(std::move(myPtr))` works fine because there will the ownership will
be transferred safely.

To clarify that there will be a transfer in ownership,
you should add `&&` to the argument.
```cpp
void myFunc(std::unique_ptr<MyClass>&& ptr){
  // Some operations
}
```
