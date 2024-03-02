# Coding Techniques

[TOC]

## Interface Class
### What is Interface Class?
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
class Interface {
public:
    virtual std::vector<double> calc(const std::vector<Particle>& particles) = 0;

    virtual ~Interface(){};
};
```
A virtual function is a member function that is declared in a base class
and is potentially overridden (redefined) in derived classes.
Declaring a virtual function as `= 0` makes it a pure virtual function,
meaning derived classes must override this function.
In the above example, all the classes that's derived from `Interface` class
***must*** override `calc` function, because it's a pure virtual function.
At the same time,
those new `calc` functions must recieve `vector<Particles>` as an input,
and return `vector<double>` as an output.
If you take a look at PressureCalculator::Implicit::calc(), for example,
you can see it follows the rule defined in this `Interface` class.
Therefore, there is no need for us to modify any other parts of the code
even when we change pressure calculation class from
`PressureCalculator::Implicit` to `PressureCalculator::Explicit`, for example.

`virtual ~Interface(){};` is a deconstructor.
It doesn't have any process in this code, but it's recommended to declare it here
by c++ rule.

So in short,
- Interface class defines the input and output of fucntions
  that its derived class must have.
- By deriving new class from the interface class, we can guarantee
  that all the derived classes have same input and output.
  Therefore there is no need for us to modify other parts of the code
  even when we switch which class to use.

### How to Use It?
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
Then it's asigned to new instance of `Dog` or `Cat`.
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

We first defined a pointer because in c++ it is the only way
to dynamically determine the entity of a variable.
Since the `Animal` class works as a class just like the `Dog` and `Cat` classes,
if you define `myAnimal` as `Animal` class,
then `myAnimal` will only work as an `Animal` class, not `Dog` or `Cat`.
If we use a pointer,
- We can change where the pointer refers,
  meaning we can determine the entity of the variable dynamically
- The compiler knows `myAnimal` is `Animal` class, allowing us to call functions
  defined in `Animal` class.