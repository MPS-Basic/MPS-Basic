# Improved Algorithms: Pressure Calculation

[TOC]

## Pressure Calculation Methods
- Implicit
- \subpage explicit

## How It's Switched
Let's take a look at how the pressure calculation method is
being modified.

### Input
First, we specify which method to use in `settings.yml`:
```yml
# pressure calculation method
pressureCalculationMethod: Implicit # Implicit or Explicit
# for Implicit
compressibility: 0.45e-09
relaxationCoefficientForPressure: 0.2
# for Explicit
soundSpeed: 17.1
```

### Loading
When executing the program, the location of `settings.yml` is passed
through the `--setting` argument.
Then main() function initiates Simulation class:
```cpp
int main(int argc, char** argv) {
    // -----
    // -----
    // -----

    auto settingPath     = fs::path(program.get<std::string>("--setting"));
    auto outputDirectory = fs::path(program.get<std::string>("--output"));
    Simulation simulation(settingPath, outputDirectory);
    simulation.run();

    return 0;
}
```

Simulation::Simulation() receives `settingPath`,
and make Loader load `settings.yml`.
User-defined pressure calculation method is now set in
`input.settings.pressureCalculationMethod`.
```cpp
Simulation::Simulation(fs::path& settingPath, fs::path& outputDirectory) {
    Input input = loader.load(settingPath, outputDirectory);

    // -----
    // -----
    // -----
}
```

### Assigning the Class
Then it moves on to switching the pressure method.
This part consists of three steps.
1. Create PressureCalculator::Interface class variable named `pressureCalculator`.
2. User-specified Pressure Calculator class (`Implicit`, `Explicit`, ...)
   will be assigned to the `pressureCalculator` variable, using `if` statements.
3. `pressureCalculator` will be handed over to MPS::MPS(),
   the constructor of MPS class. Inside MPS::MPS(),
   `pressureCalculator` will be copied to it's own public attributes.

```cpp
Simulation::Simulation(fs::path& settingPath, fs::path& outputDirectory) {
    // -----
    // -----
    // -----

    std::unique_ptr<PressureCalculator::Interface> pressureCalculator;
    if (input.settings.pressureCalculationMethod == "Implicit") {
        pressureCalculator.reset(
          new PressureCalculator::Implicit(
            // arguments for constructor
        ));

    } else if (input.settings.pressureCalculationMethod == "Explicit") {
        pressureCalculator.reset(
          new PressureCalculator::Explicit(
            // arguments for constructor
        ));
    }

    mps = MPS(input, std::move(pressureCalculator));

    // -----
    // -----
    // -----
}
```
@note
Here we use "Interface Class" and "Smart Pointer".
See [Coding Techniques](../coding_techniques.md) for more information.

### Pressure Calculation
Finally, at each time step, MPS::stepForward() calls
`pressureCalculator` that MPS class maintains, and execute pressure calculation.
```cpp
void MPS::stepForward() {
    // -----
    // -----
    // -----

    auto pressures = pressureCalculator->calc(particles);
    for (auto& particle : particles) {
        particle.pressure = pressures[particle.id];
    }

    // -----
    // -----
    // -----
}
```
